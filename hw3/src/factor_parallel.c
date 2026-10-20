//---------------------------------------
// My main driver for hw3 parallel LU factorization
//
//---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#include "layout.h"


//---------------------------------------------------------
// Unblocked parallel LU on the augmented system [A | b] like in lu.pdf
// 
// 
//
//
//---------------------------------------------------------
static void lu_unblocked(double *a, int lda, Layout L, MPI_Comm row_comm, MPI_Comm col_comm)
{   
    double *col = malloc((size_t)(L.mloc > 0 ? L.mloc : 1) * sizeof(double));
    double *row = malloc((size_t)(L.nloc > 0 ? L.nloc : 1) * sizeof(double));

    int kmax = L.m < L.n ? L.m : L.n;
    for (int k = 0; k < kmax; ++k)
    {   
        int prow = owner_row(k, L);
        int pcol = owner_col(k, L);
        
        // first local index whose global index is > k (i0, j0) or >= k (jk)
        int i0 = numroc(k+1, L.nb, L.myrow, L.P);
        int j0 = numroc(k+1, L.nb, L.mycol, L.Q);
        int jk = numroc(k,   L.nb, L.mycol, L.Q);
        
        // 1. Pivot row k (columns >= k, so the pivot rides along) goes down each process column
        if (L.myrow == prow)
        {
            int lk = local_row(k, L);
            for (int lj = jk; lj < L.nloc; ++lj)
                row[lj] = Aij(a, lda, lk, lj);
        }
        if (L.nloc - jk > 0)
            MPI_Bcast(&row[jk], L.nloc - jk, MPI_DOUBLE, prow, col_comm);
    
        // 2. Process column pcol forms the multipliers  a(i,k) <- -a(i,k)/a(k,k)
        // We assume that the pivot is nonzero because we may for this problem type.
        if (L.mycol == pcol)
        {
            int lk = local_col(k, L);
            double pivinv = 1.0 / row[lk];   // a(k,k); nonzero since A is SPD
            for (int li = i0; li < L.mloc; ++li)
            {
                double t = -pivinv * Aij(a, lda, li, lk);
                Aij(a, lda, li, lk) = t;
                col[li] = t;
            }
        }
        // 3. Multipliers go across each process row
        if (L.mloc - i0 > 0)
            MPI_Bcast(&col[i0], L.mloc - i0, MPI_DOUBLE, pcol, row_comm);
    
        // 4. Rank-1 update of my piece of the trailing matrix
        for (int lj = j0; lj < L.nloc; ++lj)
        {
            double r = row[lj];
            double *aj = &Aij(a, lda, 0, lj);
            for (int li = i0; li < L.mloc; ++li)
                aj[li] += col[li] * r;
        }
    }

    free(col);
    free(row);
}                            


// Check x = ones: with b = A*ones, column m of the eliminated [A | b] must
// equal the row sums of U over columns i..m-1.  Returns max relative error
// (valid on every rank).
static double check_ones(const double *a, int lda, Layout L, MPI_Comm row_comm)
{
    int m = L.m;
    double *rs = calloc((size_t)(L.mloc > 0 ? L.mloc : 1), sizeof(double));
    double *rhs = calloc((size_t)(L.mloc > 0 ? L.mloc : 1), sizeof(double));

    for (int lj = 0; lj < L.nloc; ++lj)
    {
        int gj = global_col(lj, L);
        for (int li = 0; li < L.mloc; ++li)
        {
            if (gj >= m)
            {
                if (gj == m) rhs[li] = Aij(a, lda, li, lj);
            }
            else if (gj >= global_row(li, L))
                rs[li] += Aij(a, lda, li, lj);
        }
    }
    // Sum partial row sums (and the rhs, nonzero only on the owner of column m)
    // across the process row.
    if (L.mloc > 0)
    {
        MPI_Allreduce(MPI_IN_PLACE, rs, L.mloc, MPI_DOUBLE, MPI_SUM, row_comm);
        MPI_Allreduce(MPI_IN_PLACE, rhs, L.mloc, MPI_DOUBLE, MPI_SUM, row_comm);
    }

    double err = 0.0, ref = 0.0;
    for (int li = 0; li < L.mloc; ++li)
    {
        double d = fabs(rs[li] - rhs[li]);
        if (d > err) err = d;
        if (fabs(rhs[li]) > ref) ref = fabs(rhs[li]);
    }
    double g[2] = {err, ref}, r[2];
    MPI_Allreduce(g, r, 2, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    free(rs);
    free(rhs);
    return r[1] > 0.0 ? r[0] / r[1] : r[0];
}

int main(int argc, char **argv)
{
    int process_Rank, size_Of_Cluster;
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    // Two modes:
    //   ./factor_parallel [nb [P Q]]        small 11 x 12 print test
    //   ./factor_parallel m n [P Q]         timed run, nb on stdin (default 1);
    //                                        prints one "RESULT ..." line
    // A leading m > 0 with n > m (two numeric args, second larger) selects
    // the timed run; the print test is chosen when fewer than 2 args are given
    // or the first arg is smaller than 8 (an nb).
    int timed = (argc >= 3 && atoi(argv[1]) >= 8);
    int m = 11, n = 12, b, P, Q;

    if (timed)
    {
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        b = 1;
        if (process_Rank == 0 && scanf("%d", &b) != 1)
            b = 1;
        MPI_Bcast(&b, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (argc >= 5)
        {
            P = atoi(argv[3]);
            Q = atoi(argv[4]);
        }
        else
        {
            int dims[2] = {0, 0};
            MPI_Dims_create(size_Of_Cluster, 2, dims);
            P = dims[0];
            Q = dims[1];
        }
    }
    else
    {
        b = (argc >= 2) ? atoi(argv[1]) : 2;
        P = (argc >= 4) ? atoi(argv[2]) : 2;
        Q = (argc >= 4) ? atoi(argv[3]) : size_Of_Cluster / P;
    }

    Layout L = build_local(m, n, b, P, Q, process_Rank, size_Of_Cluster);

    int lda = L.mloc > 1 ? L.mloc : 1;
    double *a = malloc((size_t)lda * (size_t)(L.nloc > 0 ? L.nloc : 1) * sizeof(double));

    MPI_Comm row_comm, col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, L.myrow, L.mycol, &row_comm); // ranks in my process row, ranked by mycol
    MPI_Comm_split(MPI_COMM_WORLD, L.mycol, L.myrow, &col_comm); // ranks in my process column, ranked by myrow

    spd(a, lda, L);

    if (!timed)
    {
        print_global(a, lda, "A", L, MPI_COMM_WORLD);
        lu_unblocked(a, lda, L, row_comm, col_comm);
        print_global(a, lda, "LU", L, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double t0 = MPI_Wtime();
        lu_unblocked(a, lda, L, row_comm, col_comm);
        MPI_Barrier(MPI_COMM_WORLD);
        double t = MPI_Wtime() - t0;
        double tmax;
        MPI_Reduce(&t, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        double err = (n == m + 1) ? check_ones(a, lda, L, row_comm) : -1.0;

        if (process_Rank == 0)
        {
            double gflops = 1.0e-9 * (2.0 / 3.0) * (double)m * (double)m * (double)m / tmax;
            printf("RESULT %d %d %d %d %d %d %.6e %.6f %.6f %.3e\n",
                   m, n, b, P, Q, size_Of_Cluster, tmax, gflops,
                   gflops / size_Of_Cluster, err);
            fflush(stdout);
        }
    }

    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    free(a);

    MPI_Finalize();
    return 0;
}
