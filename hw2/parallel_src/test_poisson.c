/* Rectangular Dirichlet Poisson: discrete exactness + O(h^2) convergence.
   SPMD: each rank builds and checks only its own x-row slab. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "poisson.h"
#include "msg.h"
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

static double exact_u(double x, double y, double Lx, double Ly)
{
    return sin(M_PI*x/Lx)*sin(2.0*M_PI*y/Ly);
}

/* -Laplacian of the above */
static double rhs_f(double x, double y, double Lx, double Ly)
{
    const double c = M_PI*M_PI/(Lx*Lx) + 4.0*M_PI*M_PI/(Ly*Ly);
    return c*exact_u(x,y,Lx,Ly);
}

/* Pseudo-random value in [-1,1] for global grid point (i,j), the same
   on every rank, so a rank can evaluate any entry -- including its
   neighbours' rows -- without communication.  Zero outside the grid,
   which supplies the homogeneous Dirichlet data. */
static double rand_u(const poisson_plan *p, int i, int j)
{
    uint64_t z;
    if (i<0 || i>=p->nx || j<0 || j>=p->ny) return 0.0;
    z = (uint64_t)i*0x9E3779B97F4A7C15ULL ^ ((uint64_t)j + 999)*0xC2B2AE3D27D4EB4FULL;
    z ^= z>>30; z *= 0xBF58476D1CE4E5B9ULL;           /* splitmix64 finalizer */
    z ^= z>>27; z *= 0x94D049BB133111EBULL;
    z ^= z>>31;
    return 2.0*((double)(z>>11)*(1.0/9007199254740992.0)) - 1.0;
}

/* Local slab of F = -Laplacian_h rand_u, rows xoff..xoff+m-1. */
static void rand_rhs(const poisson_plan *p, int m, int xo, double *F)
{
    const double cx=1.0/(p->hx*p->hx), cy=1.0/(p->hy*p->hy);
    int il,i,j;
    for (j=0; j<p->ny; ++j)
        for (il=0; il<m; ++il) {
            i = xo+il;
            const double c = rand_u(p,i,j);
            F[il + j*m] = cx*(2.0*c - rand_u(p,i-1,j) - rand_u(p,i+1,j))
                        + cy*(2.0*c - rand_u(p,i,j-1) - rand_u(p,i,j+1));
        }
}

int main(int argc, char **argv)
{
    int Nx=128, Ny=512, j,il;
    double Lx=2.0, Ly=1.0, e,emax,eglob;
    poisson_plan p;
    double *F,*U;

    msg_init(&argc,&argv);
    if (msg_rank()==0) printf("running on %d rank(s)\n", num_ranks());

    if (argc>1) Nx = atoi(argv[1]);
    if (argc>2) Ny = atoi(argv[2]);
    if (argc>3) Lx = atof(argv[3]);
    if (argc>4) Ly = atof(argv[4]);
    int nx=Nx-1;
    int ny=Ny-1;

    {
        const poisson_transpose_mode mode =
            (argc>5 && !strcmp(argv[5],"pairwise")) ? POISSON_TR_PAIRWISE
                                                    : POISSON_TR_ALLATONCE;
        if (poisson_plan_init(&p,nx,ny,Lx,Ly)) { msg_finalize(); return 1; }
        p.tmode = mode;
    }
    const int m  = p.mx[p.rank];      /* my x-rows */
    const int xo = p.xoff[p.rank];

    F  =(double*)malloc((size_t)m*ny*sizeof(double));
    U  =(double*)malloc((size_t)m*ny*sizeof(double));
    if (!F||!U) return 2;

    if (msg_rank()==0) {
        printf("domain  [0,%g] x [0,%g]\n",Lx,Ly);
        printf("nx ny   = %d %d      (Nx Ny = %d %d)\n",nx,ny,Nx,Ny);
        printf("hx hy   = %.6e %.6e\n",p.hx,p.hy);
        printf("transpose = %s\n",
               p.tmode==POISSON_TR_PAIRWISE ? "pairwise" : "allatonce");
    }

    /* ---- test 1: exact inversion of the DISCRETE operator ---------- */
    rand_rhs(&p,m,xo,F);
    poisson_solve(&p,F,U);
    poisson_solve(&p,F,U);
    emax=0.0;
    for (j=0; j<ny; ++j)
        for (il=0; il<m; ++il) {
            e=fabs(U[il+j*m]-rand_u(&p,xo+il,j)); if (e>emax) emax=e;
        }
    gmax_double(&emax,&eglob,1);
    if (msg_rank()==0)
        printf("\ndiscrete solve  max|U - Uexact_h|      = %.6e   (expect ~roundoff)\n",eglob);

    /* ---- test 2: continuous solution, expect O(h^2) ---------------- */
    for (j=0; j<ny; ++j) {
        double y=(double)(j+1)*p.hy;
        for (il=0; il<m; ++il) {
            double x=(double)(xo+il+1)*p.hx;
            F[il+j*m]=rhs_f(x,y,Lx,Ly);
        }
    }
    poisson_solve(&p,F,U);
    emax=0.0;
    for (j=0; j<ny; ++j) {
        double y=(double)(j+1)*p.hy;
        for (il=0; il<m; ++il) {
            double x=(double)(xo+il+1)*p.hx;
            e=fabs(U[il+j*m]-exact_u(x,y,Lx,Ly)); if (e>emax) emax=e;
        }
    }
    gmax_double(&emax,&eglob,1);
    double ep = eglob*16;
    double em = eglob/16;
    if (msg_rank()==0)
        printf("continuous      max|U - u(x,y)|        = %.6e  %.6e  %.6e  (expect O(h^2))\n",eglob,em,ep);

    poisson_plan_free(&p);
    free(F); free(U);
    msg_finalize();
    return 0;
}
