# Hand-coded version: GCC (module gcc/13.3.0)
CC = /sw/apps/gcc/13.3.0/bin/gcc

# BLAS version: same GCC, linked against FlexiBLAS (defaults to OpenBLAS).
# Select a different backend at runtime with e.g. FLEXIBLAS=NETLIB ./factor_blas
BLAS_CC   = $(CC)
BLAS_INC  = -I/usr/include/flexiblas
BLAS_LIBS = -lflexiblas

CFLAGS = -O3 -march=native -std=c11 -Wall -Wextra
BLAS_CFLAGS = $(CFLAGS) $(BLAS_INC)

all: factor factor_blas

# ------------------------------------------------------------
# Hand-coded matrix multiply
# ------------------------------------------------------------

factor: factor.o
	$(CC) $(CFLAGS) factor.o -o factor -lm

factor.o: factor.c
	$(CC) $(CFLAGS) -c factor.c -o factor.o

# ------------------------------------------------------------
# CBLAS cblas_dgemm (FlexiBLAS / OpenBLAS)
# ------------------------------------------------------------

factor_blas: factor_blas.o
	$(BLAS_CC) $(BLAS_CFLAGS) factor_blas.o -o factor_blas \
	    $(BLAS_LIBS) -lm

factor_blas.o: factor_blas.c
	$(BLAS_CC) $(BLAS_CFLAGS) -c factor_blas.c -o factor_blas.o

# ------------------------------------------------------------

clean:
	rm -f factor factor_blas *.o
