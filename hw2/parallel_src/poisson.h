#ifndef POISSON_H
#define POISSON_H
#include "block_fst.h"

/*======================================================================
 *  SPMD Dirichlet Poisson solver on the rectangle [0,Lx] x [0,Ly]:
 *
 *      -Laplacian u = f   in the interior,   u = 0 on the boundary,
 *
 *  discretized with the standard 5-point stencil.
 *
 *  GRID.  Nx = nx+1 uniform intervals in x, so
 *
 *      hx = Lx/Nx = Lx/(nx+1),   x_i = i*hx,  i = 1..nx  (interior)
 *      hy = Ly/Ny = Ly/(ny+1),   y_j = j*hy,  j = 1..ny
 *
 *  nx and ny are therefore the numbers of interior unknowns.  Each must
 *  satisfy 2*(n+1) = 4^k, so nx, ny in {31, 127, 511, 2047, ...}.
 *
 *  DECOMPOSITION.  P ranks, 1 <= P <= min(nx,ny).  Both index ranges are
 *  split into contiguous blocks, the first (n mod P) ranks getting one
 *  extra entry:
 *
 *      rows    i in [xoff[p], xoff[p]+mx[p])     (x-direction)
 *      columns j in [yoff[p], yoff[p]+my[p])     (y-direction)
 *
 *  Rank p holds two local slabs, both column-major, first index
 *  unit-stride:
 *
 *      A : mx[p] x ny,   A(il,j)  = A[il + j*mx[p]]   ("x-rows",  all y)
 *      B : my[p] x nx,   B(jl,i)  = B[jl + i*my[p]]   ("y-cols",  all x)
 *
 *  The user-facing F and U are A-layout slabs (mx[p] x ny).  With P = 1
 *  this is exactly the serial nx x ny layout.
 *
 *  METHOD.
 *
 *      A  = F                                  (mx x ny, local)
 *      A <- S_ny applied along y               (local)
 *      B  = A^T                                (distributed transpose)
 *      B <- S_nx applied along x               (local)
 *      B(jl,i) /= lam_x(i) + lam_y(yoff+jl)
 *      B <- S_nx along x                       (S is its own inverse)
 *      A  = B^T                                (distributed transpose)
 *      A <- S_ny along y
 *      U  = A
 *
 *  with the exact eigenvalues of the 5-point operator,
 *
 *      lam_x(i) = (4/hx^2) sin^2( pi*(i+1) / (2*(nx+1)) ).
 *
 *  DISTRIBUTED TRANSPOSE.  Rank p sends to rank r the block of its slab
 *  that r will own after the transpose.  Because the send side's second
 *  index is the one being redistributed, that block is a contiguous run
 *  of columns, so it goes out with no packing.  The receiver does one
 *  local transpose_real() per incoming block, writing into a contiguous
 *  run of its own output columns.  Two message schedules:
 *
 *      POISSON_TR_ALLATONCE : post all P-1 irecvs and isends, wait once.
 *      POISSON_TR_PAIRWISE  : P-1 rounds; in round k exchange with
 *                             (p+k)%P / (p-k)%P and wait before moving on.
 *====================================================================*/

typedef enum {
    POISSON_TR_ALLATONCE = 0,
    POISSON_TR_PAIRWISE  = 1
} poisson_transpose_mode;

typedef struct {
    int    nx, ny;
    double Lx, Ly, hx, hy;

    /* decomposition */
    int    P, rank;
    int    *mx, *xoff;      /* x-row   partition, length P */
    int    *my, *yoff;      /* y-col   partition, length P */

    poisson_transpose_mode tmode;   /* default POISSON_TR_ALLATONCE */
    int    verbose;                 /* default 1                    */

    block_fst_plan fx;      /* m = my[rank], n = nx : contracts along x */
    block_fst_plan fy;      /* m = mx[rank], n = ny : contracts along y */
    double *lamx, *lamy;    /* global eigenvalues                       */
    double *A, *B;          /* local slabs, mx x ny and my x nx         */
    double *R;              /* receive buffer, max(mx*ny, my*nx)        */

    /* Timing of the most recent poisson_solve, max over ranks (s).
       t_fst includes the eigenvalue division; t_tr is both transposes,
       comm plus local reshuffle; t_comm is the part of t_tr spent in
       the message calls (posting isend/irecv and msgwait). */
    double t_total, t_fst, t_tr, t_comm;
    double gflops;
} poisson_plan;

/* Collective.  Must be called after msg_init(). */
int  poisson_plan_init(poisson_plan *p,
                            int nx, int ny, double Lx, double Ly);
void poisson_plan_free(poisson_plan *p);

/* Collective.  F and U are this rank's mx[rank] x ny slabs; U may
   alias F.  Fills p->t_* and p->gflops; rank 0 prints a summary line
   if p->verbose. */
void poisson_solve(poisson_plan *p,
                        const double * restrict F, double * restrict U);

/* Apply the 5-point operator -Laplacian_h, homogeneous Dirichlet, to a
   FULL nx x ny array (serial; used only to build test data). */
void poisson_residual_op(const poisson_plan *p,
                              const double * restrict U,
                              double * restrict F);
#endif
