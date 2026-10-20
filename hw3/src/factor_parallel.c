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


int main(int argc, char **argv)
{
    int process_Rank, size_Of_Cluster;
    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    
    int m = 11, n = 12;
    int b = (argc >= 2) ? atoi(argv[1]) : 2;
    int P = (argc >= 4) ? atoi(argv[2]) : 2;
    int Q = (argc >= 4) ? atoi(argv[3]) : size_Of_Cluster / P;

    Layout L = build_local(m, n, b, P, Q, process_Rank, size_Of_Cluster);
    
    int lda = L.mloc > 1 ? L.mloc : 1;
    double *a = malloc((size_t)lda * (size_t)(L.nloc > 0 ? L.nloc : 1) * sizeof(double));

    MPI_Comm row_comm, col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, L.myrow, L.mycol, &row_comm); // ranks in my process row, ranked by mycol
    MPI_Comm_split(MPI_COMM_WORLD, L.mycol, L.myrow, &col_comm); // ranks in my process column, ranked by myrow

    spd(a, lda, L);
    print_global(a, lda, "A", L, MPI_COMM_WORLD);

    lu_unblocked(a, lda, L, row_comm, col_comm);
    print_global(a, lda, "LU", L, MPI_COMM_WORLD);

    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);
    free(a);

    MPI_Finalize();
    return 0;
}  

