//---------------------------------------
// Q2: BLAS3 blocked parallel LU (no pivoting) on a 2D block-cyclic layout
//
// Distribution block size nb = pivot block size b.  Same driver, test
// matrix and RESULT line as Q1 (../src/factor_parallel.c).
//---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#include "layout.h"

#ifdef USE_CBLAS
#include <cblas.h>
#endif

//---------------------------------------------------------
// Local kernels.  With -DUSE_CBLAS (BLAS=blis|mkl) these call the vendor
// BLAS; otherwise plain loops, so the code can be tested without a BLAS.
// All matrices are column-major.
//---------------------------------------------------------

// C(m x n) -= A(m x k) * B(k x n)
static void gemm_sub(int m, int n, int k, const double *A, int lda,
                     const double *B, int ldb, double *C, int ldc)
{
#ifdef USE_CBLAS
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, n, k,
                -1.0, A, lda, B, ldb, 1.0, C, ldc);
#else
    for (int j = 0; j < n; ++j)
        for (int p = 0; p < k; ++p)
        {
            double bpj = Aij(B, ldb, p, j);
            for (int i = 0; i < m; ++i)
                Aij(C, ldc, i, j) -= Aij(A, lda, i, p) * bpj;
        }
#endif
}

// B(m x n) <- B * U^-1, U (n x n) upper triangular, non-unit diagonal
static void trsm_right_upper(int m, int n, const double *U, int ldu, double *B, int ldb)
{
#ifdef USE_CBLAS
    cblas_dtrsm(CblasColMajor, CblasRight, CblasUpper, CblasNoTrans, CblasNonUnit,
                m, n, 1.0, U, ldu, B, ldb);
#else
    for (int j = 0; j < n; ++j)
    {
        for (int k = 0; k < j; ++k)
        {
            double ukj = Aij(U, ldu, k, j);
            for (int i = 0; i < m; ++i)
                Aij(B, ldb, i, j) -= Aij(B, ldb, i, k) * ukj;
        }
        double inv = 1.0 / Aij(U, ldu, j, j);
        for (int i = 0; i < m; ++i)
            Aij(B, ldb, i, j) *= inv;
    }
#endif
}

// B(m x n) <- L^-1 * B, L (m x m) lower triangular, unit diagonal
static void trsm_left_lower_unit(int m, int n, const double *Lm, int ldl, double *B, int ldb)
{
#ifdef USE_CBLAS
    cblas_dtrsm(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans, CblasUnit,
                m, n, 1.0, Lm, ldl, B, ldb);
#else
    for (int j = 0; j < n; ++j)
        for (int k = 0; k < m; ++k)
        {
            double bkj = Aij(B, ldb, k, j);
            for (int i = k + 1; i < m; ++i)
                Aij(B, ldb, i, j) -= Aij(Lm, ldl, i, k) * bkj;
        }
#endif
}

// In-place LU of a small nb x nb block, no pivoting; standard (positive) L
static void lu_small(double *A, int lda, int nb)
{
    for (int k = 0; k < nb; ++k)
    {
        double inv = 1.0 / Aij(A, lda, k, k);
        for (int i = k + 1; i < nb; ++i)
            Aij(A, lda, i, k) *= inv;
        for (int j = k + 1; j < nb; ++j)
        {
            double r = Aij(A, lda, k, j);
            for (int i = k + 1; i < nb; ++i)
                Aij(A, lda, i, j) -= Aij(A, lda, i, k) * r;
        }
    }
}

//---------------------------------------------------------
// Blocked parallel LU on the augmented system [A | b], block size L.nb.
// For each diagonal block k:
//   1. owner factors A11 = L11 U11, broadcasts it along its process row/column
//   2. process column: L21 = A21 U11^-1;  process row: U12 = L11^-1 A12
//   3. L21 goes across process rows, U12 down process columns
//   4. everyone updates A22 -= L21 * U12  (dgemm)
// Leaves standard L (unit diagonal implied) below the diagonal and U on/above;
// negate_lower() converts to the -L convention used by Q1 for printing.
//---------------------------------------------------------
static void lu_blocked(double *a, int lda, Layout L, MPI_Comm row_comm, MPI_Comm col_comm)
{
    int b = L.nb;
    double *d    = malloc((size_t)b * b * sizeof(double));
    double *Lbuf = malloc((size_t)(L.mloc > 0 ? L.mloc : 1) * b * sizeof(double));
    double *Ubuf = malloc((size_t)b * (size_t)(L.nloc > 0 ? L.nloc : 1) * sizeof(double));

    int kmax = L.m < L.n ? L.m : L.n;
    for (int kk = 0; kk < kmax; kk += b)
    {
        int nbk = (kmax - kk < b) ? kmax - kk : b;
        int prow = owner_row(kk, L);
        int pcol = owner_col(kk, L);

        // first local row/col past this block
        int i0 = numroc(kk + nbk, b, L.myrow, L.P);
        int j0 = numroc(kk + nbk, b, L.mycol, L.Q);
        int mrem = L.mloc - i0;
        int nrem = L.nloc - j0;

        int lr = local_row(kk, L);
        int lc = local_col(kk, L);

        // 1. factor the diagonal block and broadcast it
        if (L.myrow == prow && L.mycol == pcol)
        {
            lu_small(&Aij(a, lda, lr, lc), lda, nbk);
            for (int j = 0; j < nbk; ++j)
                for (int i = 0; i < nbk; ++i)
                    d[i + (size_t)nbk * j] = Aij(a, lda, lr + i, lc + j);
        }
        if (L.mycol == pcol)
            MPI_Bcast(d, nbk * nbk, MPI_DOUBLE, prow, col_comm);
        if (L.myrow == prow)
            MPI_Bcast(d, nbk * nbk, MPI_DOUBLE, pcol, row_comm);

        // 2. triangular solves
        if (L.mycol == pcol && mrem > 0)
            trsm_right_upper(mrem, nbk, d, nbk, &Aij(a, lda, i0, lc), lda);
        if (L.myrow == prow && nrem > 0)
            trsm_left_lower_unit(nbk, nrem, d, nbk, &Aij(a, lda, lr, j0), lda);

        // 3. broadcast L21 across process rows, U12 down process columns
        if (mrem > 0)
        {
            if (L.mycol == pcol)
                for (int j = 0; j < nbk; ++j)
                    for (int i = 0; i < mrem; ++i)
                        Lbuf[i + (size_t)mrem * j] = Aij(a, lda, i0 + i, lc + j);
            MPI_Bcast(Lbuf, mrem * nbk, MPI_DOUBLE, pcol, row_comm);
        }
        if (nrem > 0)
        {
            if (L.myrow == prow)
                for (int j = 0; j < nrem; ++j)
                    for (int i = 0; i < nbk; ++i)
                        Ubuf[i + (size_t)nbk * j] = Aij(a, lda, lr + i, j0 + j);
            MPI_Bcast(Ubuf, nbk * nrem, MPI_DOUBLE, prow, col_comm);
        }

        // 4. trailing update
        if (mrem > 0 && nrem > 0)
            gemm_sub(mrem, nrem, nbk, Lbuf, mrem, Ubuf, nbk, &Aij(a, lda, i0, j0), lda);
    }

    free(d);
    free(Lbuf);
    free(Ubuf);
}

// Standard L -> -L (strictly lower part), matching lu_unblocked's storage
static void negate_lower(double *a, int lda, Layout L)
{
    for (int lj = 0; lj < L.nloc; ++lj)
    {
        int gj = global_col(lj, L);
        for (int li = 0; li < L.mloc; ++li)
            if (global_row(li, L) > gj)
                Aij(a, lda, li, lj) = -Aij(a, lda, li, lj);
    }
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
    //   ./factor_blocked [nb [P Q]]        small 11 x 12 print test
    //   ./factor_blocked m n [P Q]         timed run, nb on stdin (default 1);
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
        lu_blocked(a, lda, L, row_comm, col_comm);
        negate_lower(a, lda, L);
        print_global(a, lda, "LU", L, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Barrier(MPI_COMM_WORLD);
        double t0 = MPI_Wtime();
        lu_blocked(a, lda, L, row_comm, col_comm);
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
