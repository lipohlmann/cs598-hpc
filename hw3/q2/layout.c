//---------------------------------------
// 2D block-cyclic layout helpers for hw3
//---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "layout.h"

int numroc(int n, int nb, int iproc, int nprocs)
{
    int nblocks = n / nb;                      // full blocks
    int num = (nblocks / nprocs) * nb;         // full rounds of the deal
    int extra = nblocks % nprocs;              // leftover full blocks
    if (iproc < extra)
        num += nb;
    else if (iproc == extra)
        num += n % nb;                         // the trailing partial block
    return num;
}

int global_row(int li, Layout L)
{
    return ((li / L.nb) * L.P + L.myrow) * L.nb + li % L.nb;
}

int global_col(int lj, Layout L)
{
    return ((lj / L.nb) * L.Q + L.mycol) * L.nb + lj % L.nb;
}

int owner_row(int gi, Layout L) { return (gi / L.nb) % L.P; }
int owner_col(int gj, Layout L) { return (gj / L.nb) % L.Q; }

int local_row(int gi, Layout L)
{
    return (gi / (L.nb * L.P)) * L.nb + gi % L.nb;
}

int local_col(int gj, Layout L)
{
    return (gj / (L.nb * L.Q)) * L.nb + gj % L.nb;
}

Layout build_local(int m, int n, int b, int P, int Q, int rank, int size_Of_Cluster)
{
    if (P*Q != size_Of_Cluster)
    {
        if (rank == 0)
            fprintf(stderr, "Invalid choice of P=%d Q=%d for %d ranks\n", P, Q, size_Of_Cluster);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    Layout L;
    L.myrow = rank % P;  L.mycol = rank / P; // Column major
    L.m = m;
    L.n = n;
    L.nb = b;
    L.P = P;
    L.Q = Q;

    L.mloc = numroc(m, b, L.myrow, P);
    L.nloc = numroc(n, b, L.mycol, Q);

    return L;
}

void spd(double *a, int lda, Layout L)
{
    const double pi = 4.0*atan(1.0);
    const int N = L.m + 1;
    const double scale = sqrt(2.0 / (double)N);

    for (int lj = 0; lj < L.nloc; ++lj)
    {
        int gj = global_col(lj, L);
        for (int li = 0; li < L.mloc; ++li)
        {
            int i = global_row(li, L) + 1;
            double v;
            if (gj < L.m)                      // A part
            {
                int j = gj + 1;
                v = scale * sin(pi * (double)i * (double)j / (double)N);
                if (i == j)
                    v += (double)N;            // a = N on the diagonal
            }
            else                               // extra columns: b = A * ones
            {
                v = (double)N;
                for (int j = 1; j <= L.m; ++j)
                    v += scale * sin(pi * (double)i * (double)j / (double)N);
            }
            Aij(a, lda, li, lj) = v;
        }
    }
}

void print_global(const double *a, int lda, const char *name, Layout L, MPI_Comm comm)
{
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // Pack my local block contiguously (column-major, ld = mloc)
    int mycount = L.mloc * L.nloc;
    double *sendbuf = malloc((size_t)(mycount > 0 ? mycount : 1) * sizeof(double));
    for (int lj = 0; lj < L.nloc; ++lj)
        for (int li = 0; li < L.mloc; ++li)
            sendbuf[li + (size_t)L.mloc * lj] = Aij(a, lda, li, lj);

    if (rank != 0)
    {
        MPI_Send(sendbuf, mycount, MPI_DOUBLE, 0, 0, comm);
        free(sendbuf);
        return;
    }

    int m = L.m, n = L.n;
    double *g = malloc((size_t)m * (size_t)n * sizeof(double));
    double *recvbuf = NULL;

    for (int r = 0; r < size; ++r)
    {
        // Rebuild rank r's layout so we know where its entries go
        Layout Ls = L;
        Ls.myrow = r % L.P;  Ls.mycol = r / L.P;   // Column major, same as build_local
        Ls.mloc = numroc(m, L.nb, Ls.myrow, L.P);
        Ls.nloc = numroc(n, L.nb, Ls.mycol, L.Q);
        int count = Ls.mloc * Ls.nloc;

        const double *src;
        if (r == 0)
            src = sendbuf;
        else
        {
            recvbuf = realloc(recvbuf, (size_t)(count > 0 ? count : 1) * sizeof(double));
            MPI_Recv(recvbuf, count, MPI_DOUBLE, r, 0, comm, MPI_STATUS_IGNORE);
            src = recvbuf;
        }

        for (int lj = 0; lj < Ls.nloc; ++lj)
            for (int li = 0; li < Ls.mloc; ++li)
                Aij(g, m, global_row(li, Ls), global_col(lj, Ls)) = src[li + (size_t)Ls.mloc * lj];
    }

    printf("%s (%d x %d, nb=%d, P=%d, Q=%d):\n", name, m, n, L.nb, L.P, L.Q);
    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < n; ++j)
            printf(" %9.4f", Aij(g, m, i, j));
        printf("\n");
    }
    fflush(stdout);

    free(g);
    free(recvbuf);
    free(sendbuf);
}
