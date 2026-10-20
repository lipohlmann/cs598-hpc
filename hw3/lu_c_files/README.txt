C block LU factorization test
=============================

factor.c       Hand-written matrix-update kernel.
factor_blas.c  Same, but the update C <- C + A*B uses cblas_dgemm
               (FlexiBLAS/OpenBLAS on the cluster, Accelerate on macOS).
doall          Runs factor over several block sizes.
doall_blas     Runs factor_blas over several block sizes.

Build:   make
Run:     ./doall   or   ./doall_blas
One run: echo 32 | ./factor_blas 4000 4001     (block size on stdin; optional m n)

Arrays are 1-D, column-major: A(i,j) = a[i + lda*j].
