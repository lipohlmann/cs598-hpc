//---------------------------------------
// 2D block-cyclic layout helpers for hw3
//---------------------------------------

#ifndef LAYOUT_H
#define LAYOUT_H

#include <stddef.h>
#include <mpi.h>

#define Aij(a,ld,i,j) ((a)[(size_t)(i) + (size_t)(ld)*(size_t)(j)])

typedef struct
{
    int m, n, nb;       // global rows, cols, block size
    int P, Q;           // grid shape
    int myrow, mycol;   // my grid position
    int mloc, nloc;     // my local rows/cols (lda = mloc)
} Layout;

// Rows/cols of an n-length dimension owned by process coordinate iproc
int numroc(int n, int nb, int iproc, int nprocs);

// local -> global
int global_row(int li, Layout L);
int global_col(int lj, Layout L);

// global -> owning process row/col
int owner_row(int gi, Layout L);
int owner_col(int gj, Layout L);

// global -> local (only meaningful on the owner)
int local_row(int gi, Layout L);
int local_col(int gj, Layout L);

Layout build_local(int m, int n, int b, int P, int Q, int rank, int size_Of_Cluster);

// Fill my local piece of [A | b]: A = S + N*I (m x m) from hw3.pdf,
// and any extra columns with b = A * ones
void spd(double *a, int lda, Layout L);

// Gather to rank 0 and print.  Every rank must call this.  Small n only.
void print_global(const double *a, int lda, const char *name, Layout L, MPI_Comm comm);

#endif
