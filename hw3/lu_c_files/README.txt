C block-factorization test
==========================

Files
-----

doall          Runs the hand-written version over several block sizes.
doall_blas     Runs the Accelerate/CBLAS version over the same block sizes.


factor.c       Hand-written matrix-update kernel (including b=8 specialization).
factor_blas.c  Same factorization, but the Schur update C <- C + A*B uses
               cblas_dgemm() from Apple's Accelerate framework.

Makefile       Builds both variants.

               **NOTE** This is the correct makefile for the Mac

There are also BLAS1 type variants, etc.

Build
-----
    make

Run
---
    ./doall
    ./doall_blas

or, for one case:
    echo 8 | ./factor
    echo 8 | ./factor_blas

Optional dimensions:
    echo 32 | ./factor_blas 4000 4001

Storage
-------
Both versions use one-dimensional double arrays with column-major indexing:
    a[i + lda*j]
so the layout agrees with the original Fortran matrix storage.

BLAS on macOS
-------------
The BLAS variant includes <Accelerate/Accelerate.h> and links with:
    -framework Accelerate
No separate BLAS installation is required on macOS.
