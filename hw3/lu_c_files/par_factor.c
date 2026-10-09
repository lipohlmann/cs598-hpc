/* Block LU factorization test using CBLAS DGEMM, translated from factor.f.
 *
 * Storage is one-dimensional, column-major, so that
 *     A(i,j) <-> a[i + lda*j]       (0-based C indices)
 * This preserves the memory-access pattern of the Fortran code.
 *
 * For each diagonal block A:
 *     A -> L U
 *     C -> -C U^{-1}
 *     B ->  L^{-1} B
 *     D -> D + C B
 *
 * No pivoting is performed, matching factor.f.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <Accelerate/Accelerate.h>

#define Aij(a,ld,i,j) ((a)[(size_t)(i) + (size_t)(ld)*(size_t)(j)])

static double cpu_time(void)
{
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static void randm(double *a, int lda, int m, int n)
{
    const double pi = 4.0*atan(1.0);

    for (int j=0; j<n; ++j)
        for (int i=0; i<m; ++i)
            Aij(a,lda,i,j) = cos(pi*(double)(i+1)*(double)(j+1)/(double)m);

    /* Intended diagonal shift.  This avoids the uninitialized-diagonal
       issue in the original randm() routine. */
    const int kmax = (m < n) ? m : n;
    for (int i=0; i<kmax; ++i)
        Aij(a,lda,i,i) += 5.0;
}

static void outmat(const double *a, int lda, int m, int n,
                   const char *name, int ie)
{
    char filename[64];
    snprintf(filename,sizeof(filename),"%.6s%d",name,abs(ie)%10);

    FILE *fp = fopen(filename,"w");
    if (!fp) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    printf("\n%d matrix: %.6s %d %d\n",ie,name,m,n);
    const int nout = (n < 20) ? n : 20;
    for (int i=0; i<m; ++i) {
        for (int j=0; j<nout; ++j)
            fprintf(fp," %18.9e",Aij(a,lda,i,j));
        fputc('\n',fp);
    }
    putchar('\n');
    fclose(fp);
}

/* C = C + A*B, all matrices stored column-major.
 * A is m x l with leading dimension lda.
 * B is l x n with leading dimension ldb.
 * C is m x n with leading dimension ldc.
 */
/* C = C + A*B using optimized CBLAS DGEMM from Apple Accelerate.
 * All matrices are column-major, matching the original Fortran storage.
 * A is m x l, B is l x n, C is m x n.
 */
static void mxma(const double *a, int lda, int m,
                 const double *b, int ldb, int l,
                 double *c, int ldc, int n)
{
    if (m<=0 || l<=0 || n<=0) return;

    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
                m, n, l,
                1.0, a, lda,
                     b, ldb,
                1.0, c, ldc);
}

static int block_factor(double *a, int lda, int m, int n, int bs,
                        double *mxm_time)
{
    const int kmax = (m < n) ? m : n;
    double *col = malloc((size_t)m*sizeof(*col));
    double *row = malloc((size_t)n*sizeof(*row));
    if (!col || !row) {
        fprintf(stderr,"workspace allocation failed\n");
        free(col); free(row);
        return -1;
    }

    *mxm_time = 0.0;

    for (int kk=0; kk<kmax; kk += bs) {
        /* Actual size of this diagonal block.  This is important when
           bs does not divide min(m,n). */
        const int nb = ((kmax-kk) < bs) ? (kmax-kk) : bs;
        const int kend = kk + nb;       /* first row/column after block */

        for (int kl=0; kl<nb; ++kl) {
            const int k = kk + kl;
            const double pivot = Aij(a,lda,k,k);
            if (pivot == 0.0) {
                fprintf(stderr,"zero pivot at k=%d\n",k+1);
                free(col); free(row);
                return k+1;
            }
            const double pivinv = 1.0/pivot;

            for (int i=k+1; i<m; ++i) {                   /* C = - multiplier column */
                const double tmp = -pivinv*Aij(a,lda,i,k);
                col[i] = tmp;
                Aij(a,lda,i,k) = tmp;
            }

            for (int j=k+1; j<n; ++j)                     /* Save pivot row. */
                row[j] = Aij(a,lda,k,j);

            for (int j=k+1; j<n; ++j) {                   /* Factor upper and left panels */
                const int imax = (j < kend) ? m : kend;
                for (int i=k+1; i<imax; ++i)
                    Aij(a,lda,i,j) += col[i]*row[j];
            }
        }

        const int mloc = m-kend;
        const int nloc = n-kend;
        if (mloc>0 && nloc>0) {
            const double s0 = cpu_time();
            mxma(&Aij(a,lda,kend,kk), lda, mloc,
                 &Aij(a,lda,kk,kend), lda, nb,
                 &Aij(a,lda,kend,kend), lda, nloc);
            *mxm_time += cpu_time()-s0;
        }
    }

    free(col);
    free(row);
    return 0;
}

int main(int argc, char **argv)
{
    int m = 1803;
    int n = 1804;
    int bs;

    /* Optional dimensions make larger tests easy:
         ./factor 4000 4001 < t.dat
       Block size is still read from stdin, as in factor.f/doall. */
    if (argc >= 2) m = atoi(argv[1]);
    if (argc >= 3) n = atoi(argv[2]);
    if (m<=0 || n<=0) {
        fprintf(stderr,"usage: %s [m [n]]\n",argv[0]);
        return EXIT_FAILURE;
    }

    printf("Input blocksize, b:\n");
    if (scanf("%d",&bs) != 1 || bs<=0) {
        fprintf(stderr,"invalid block size\n");
        return EXIT_FAILURE;
    }

    const int lda = m;
    const size_t mn = (size_t)m*(size_t)n;
    double *a = malloc(mn*sizeof(*a));
    if (!a) {
        fprintf(stderr,"unable to allocate %.3f MB\n",
                (double)(mn*sizeof(*a))/(1024.0*1024.0));
        return EXIT_FAILURE;
    }

    randm(a,lda,m,n);
    outmat(a,lda,m,n,"starta",0);

    double mxm_time = 0.0;
    const double t0 = cpu_time();
    const int ierr = block_factor(a,lda,m,n,bs,&mxm_time);
    const double tm = cpu_time()-t0;
    if (ierr) {
        free(a);
        return EXIT_FAILURE;
    }

    /* Same square-matrix estimate used by factor.f. */
    const double flops = (2.0/3.0)*(double)m*(double)m*(double)m;
    const double gflops = 1.0e-9*flops/tm;

    printf("%4d %9d %9.2f  %12.4e %12.4e %12.4e"
           " b,n,gflops,CPU time, mxm time, ratio\n",
           bs,n,gflops,tm,mxm_time,mxm_time/tm);

    outmat(a,lda,m,n,"finalc",(m<n)?m:n);
    free(a);
    return 0;
}
