#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "poisson.h"
#include "transpose.h"
#include "msg.h"
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define TAG_XY 101
#define TAG_YX 102

/* Contiguous block partition of 0..n-1 over P ranks; the first n%P
   ranks get one extra entry. */
static void partition(int n, int P, int *cnt, int *off)
{
    const int base=n/P, rem=n%P;
    int r;
    off[0]=0;
    for (r=0; r<P; ++r) {
        cnt[r] = base + (r<rem ? 1 : 0);
        if (r>0) off[r] = off[r-1] + cnt[r-1];
    }
}

int poisson_plan_init(poisson_plan *p,
                           int nx, int ny, double Lx, double Ly)
{
    int i,j,P,me;

    memset(p,0,sizeof(*p));   /* so poisson_plan_free is safe on any error */
    p->nx=nx; p->ny=ny; p->Lx=Lx; p->Ly=Ly;
    p->hx = Lx/(double)(nx+1);
    p->hy = Ly/(double)(ny+1);
    p->tmode   = POISSON_TR_ALLATONCE;
    p->verbose = 1;

    p->P    = P  = num_ranks();
    p->rank = me = msg_rank();
    if (P>nx || P>ny) {
        if (me==0)
            fprintf(stderr,"poisson: need P <= min(nx,ny); P=%d nx=%d ny=%d\n",
                    P,nx,ny);
        return 1;
    }

    p->mx   = (int*) malloc((size_t)P*sizeof(int));
    p->xoff = (int*) malloc((size_t)P*sizeof(int));
    p->my   = (int*) malloc((size_t)P*sizeof(int));
    p->yoff = (int*) malloc((size_t)P*sizeof(int));
    if (!p->mx || !p->xoff || !p->my || !p->yoff) { poisson_plan_free(p); return 2; }
    partition(nx,P,p->mx,p->xoff);
    partition(ny,P,p->my,p->yoff);

    /* fy contracts along y: local slab is mx x ny, batch = mx. */
    if (block_fst_plan_init(&p->fy,p->mx[me],ny)) { poisson_plan_free(p); return 1; }
    /* fx contracts along x: local slab is my x nx, batch = my. */
    if (block_fst_plan_init(&p->fx,p->my[me],nx)) { poisson_plan_free(p); return 1; }

    {
        const size_t na = (size_t)p->mx[me]*ny;
        const size_t nb = (size_t)p->my[me]*nx;
        p->lamx = (double*) malloc((size_t)nx*sizeof(double));
        p->lamy = (double*) malloc((size_t)ny*sizeof(double));
        p->A    = (double*) malloc(na*sizeof(double));
        p->B    = (double*) malloc(nb*sizeof(double));
        p->R    = (double*) malloc((na>nb ? na : nb)*sizeof(double));
    }
    if (!p->lamx || !p->lamy || !p->A || !p->B || !p->R) {
        poisson_plan_free(p); return 2;
    }

    /* Exact eigenvalues of the 5-point operator. */
    for (i=0; i<nx; ++i) {
        double s = sin(0.5*M_PI*(double)(i+1)/(double)(nx+1));
        p->lamx[i] = 4.0*s*s/(p->hx*p->hx);
    }
    for (j=0; j<ny; ++j) {
        double s = sin(0.5*M_PI*(double)(j+1)/(double)(ny+1));
        p->lamy[j] = 4.0*s*s/(p->hy*p->hy);
    }
    return 0;
}

void poisson_plan_free(poisson_plan *p)
{
    block_fst_plan_free(&p->fx);
    block_fst_plan_free(&p->fy);
    free(p->mx);   p->mx=0;
    free(p->xoff); p->xoff=0;
    free(p->my);   p->my=0;
    free(p->yoff); p->yoff=0;
    free(p->lamx); p->lamx=0;
    free(p->lamy); p->lamy=0;
    free(p->A);    p->A=0;
    free(p->B);    p->B=0;
    free(p->R);    p->R=0;
}

/*----------------------------------------------------------------------
 *  Distributed transpose.
 *
 *  in  : m x N2 local slab, in(il,c) = in[il + c*m].  The second index
 *        (global length N2) is what gets redistributed: rank r will own
 *        columns [os[r], os[r]+cs[r]).
 *  out : q x N1 local slab, q = cs[me], out(cl,g) = out[cl + g*q].  The
 *        first index of `in` is partitioned as (cr, or), with cr[me]==m,
 *        and becomes out's second (global) index.
 *
 *  To rank r we send columns os[r].. of `in`: contiguous, m*cs[r] words.
 *  From rank s we receive an cr[s] x q block, which lands (transposed)
 *  in out columns or[s].. -- also contiguous, at out + or[s]*q.  The
 *  receive buffer uses the same offsets, so R + or[s]*q.
 *
 *  Returns the time this rank spent in the message calls (posting the
 *  isends/irecvs and waiting on them), excluding the local transposes.
 *--------------------------------------------------------------------*/
static double dist_transpose(poisson_plan *p,
                           const double * restrict in, double * restrict out,
                           int m, const int *cs, const int *os,
                                  const int *cr, const int *or_, int tag)
{
    const int P=p->P, me=p->rank, q=cs[me];
    const int dsz=(int)sizeof(double);
    double * restrict R = p->R;
    double ts, tc=0.0;
    int k,r,s;

    if (p->tmode == POISSON_TR_ALLATONCE) {
        ts=msg_wtime();
        /* Post every receive first so sends can land directly. */
        for (k=1; k<P; ++k) {
            s = (me-k+P)%P;
            irecv(s, R + (size_t)or_[s]*q, cr[s]*q*dsz, tag);
        }
        /* Stagger destinations so ranks don't all hit rank 0 first. */
        for (k=1; k<P; ++k) {
            r = (me+k)%P;
            isend(r, in + (size_t)os[r]*m, m*cs[r]*dsz, tag);
        }
        tc+=msg_wtime()-ts;
        /* Own block overlaps with the communication. */
        transpose_real(m,q, in + (size_t)os[me]*m, out + (size_t)or_[me]*q);
        ts=msg_wtime();
        msgwait();
        tc+=msg_wtime()-ts;
        for (k=1; k<P; ++k) {
            s = (me-k+P)%P;
            transpose_real(cr[s],q, R + (size_t)or_[s]*q, out + (size_t)or_[s]*q);
        }
    } else {
        transpose_real(m,q, in + (size_t)os[me]*m, out + (size_t)or_[me]*q);
        /* Round k: send to me+k, receive from me-k.  Every rank has
           exactly one partner in each direction per round. */
        for (k=1; k<P; ++k) {
            r = (me+k)%P;
            s = (me-k+P)%P;
            ts=msg_wtime();
            irecv(s, R + (size_t)or_[s]*q, cr[s]*q*dsz, tag);
            isend(r, in + (size_t)os[r]*m, m*cs[r]*dsz, tag);
            msgwait();
            tc+=msg_wtime()-ts;
            transpose_real(cr[s],q, R + (size_t)or_[s]*q, out + (size_t)or_[s]*q);
        }
    }
    return tc;
}

void poisson_solve(poisson_plan *p,
                        const double * restrict F, double * restrict U)
{
    const int nx=p->nx, ny=p->ny, me=p->rank;
    const int mxl=p->mx[me], myl=p->my[me], yo=p->yoff[me];
    const size_t na=(size_t)mxl*ny;
    double * restrict A = p->A;
    double * restrict B = p->B;
    double ts, tf=0.0, tt=0.0, tc=0.0;
    size_t k;
    int i,j;

    for (k=0; k<na; ++k) A[k]=F[k];

    msg_barrier();   /* start everyone together; not timed */

    /* ---- timed region: the actual FST solve, steps 1-7 -------------- */
    const double t0 = msg_wtime();

    /* 1. transform along y: A is mx x ny, contract over the ny index */
    ts=msg_wtime();
    block_fst_apply(&p->fy,A);
    tf+=msg_wtime()-ts;

    /* 2. A(mx x ny, x-rows) -> B(my x nx, y-cols) */
    ts=msg_wtime();
    tc+=dist_transpose(p,A,B, mxl, p->my,p->yoff, p->mx,p->xoff, TAG_XY);
    tt+=msg_wtime()-ts;

    ts=msg_wtime();
    /* 3. transform along x: B is my x nx, contract over the nx index */
    block_fst_apply(&p->fx,B);

    /* 4. divide by the eigenvalues;  B(jl,i) = B[jl + i*my] */
    for (i=0; i<nx; ++i) {
        const double lx = p->lamx[i];
        const double * restrict ly = &p->lamy[yo];
        double * restrict Bi = &B[(size_t)i*myl];
        for (j=0; j<myl; ++j)
            Bi[j] /= (lx + ly[j]);
    }

    /* 5. back along x (S is its own inverse) */
    block_fst_apply(&p->fx,B);
    tf+=msg_wtime()-ts;

    /* 6. B(my x nx) -> A(mx x ny) */
    ts=msg_wtime();
    tc+=dist_transpose(p,B,A, myl, p->mx,p->xoff, p->my,p->yoff, TAG_YX);
    tt+=msg_wtime()-ts;

    /* 7. back along y */
    ts=msg_wtime();
    block_fst_apply(&p->fy,A);
    tf+=msg_wtime()-ts;

    const double t1 = msg_wtime();
    /* ---- end timed region -------------------------------------------- */

    {
        const double loc[4] = { t1-t0, tf, tt, tc };
        double glob[4];
        gmax_double(loc,glob,4);
        p->t_total=glob[0]; p->t_fst=glob[1]; p->t_tr=glob[2];
        p->t_comm=glob[3];
    }
    {
        const int    Nx = nx+1, Ny = ny+1;
        /* 4 batched-FST passes (steps 1,3,5,7), each O(m n log2 n);
         * the crude count below folds all four into one estimate:
         * flops ~ 2*10*nx*ny*(log2(Nx)+log2(Ny)).  Global count, so
         * this is the aggregate rate over all P ranks.  Nominal: the
         * FST really runs a complex FFT of length 2(n+1). */
        const double flops  = 2.0*10.0*(double)nx*(double)ny*
                               (log2((double)Nx) + log2((double)Ny));
        p->gflops = (flops/p->t_total)/1e9;
        if (p->verbose && me==0)
            printf("poisson_solve: P %5d  Nx %8d  Ny %8d  %-9s  elapsed %12.6f s"
                   "  (fst %10.6f  transpose %10.6f  comm %10.6f)  GFLOPS %8.3f\n",
                   p->P, Nx, Ny,
                   p->tmode==POISSON_TR_PAIRWISE ? "pairwise" : "allatonce",
                   p->t_total, p->t_fst, p->t_tr, p->t_comm, p->gflops);
    }

    for (k=0; k<na; ++k) U[k]=A[k];
}

/* F = -Laplacian_h U on the FULL nx x ny array, zero Dirichlet data
   outside the index range. */
void poisson_residual_op(const poisson_plan *p,
                              const double * restrict U,
                              double * restrict F)
{
    const int nx=p->nx, ny=p->ny;
    const double cx=1.0/(p->hx*p->hx), cy=1.0/(p->hy*p->hy);
    int i,j;

    for (j=0; j<ny; ++j) {
        for (i=0; i<nx; ++i) {
            const double c  = U[i + j*nx];
            const double w  = (i>0)    ? U[(i-1) + j*nx] : 0.0;
            const double e  = (i<nx-1) ? U[(i+1) + j*nx] : 0.0;
            const double s  = (j>0)    ? U[i + (j-1)*nx] : 0.0;
            const double nn = (j<ny-1) ? U[i + (j+1)*nx] : 0.0;
            F[i + j*nx] = cx*(2.0*c - w - e) + cy*(2.0*c - s - nn);
        }
    }
}
