# Hand-coded version: Homebrew GCC
CC = /opt/homebrew/bin/gcc-15

# Apple Accelerate version: Apple Clang
ACCEL_CC = /usr/bin/clang

CFLAGS = -O3 -march=native -std=c11 -Wall -Wextra
ACCEL_CFLAGS = -O3 -march=native -std=c11 -Wall -Wextra \
               -DACCELERATE_NEW_LAPACK

all: factor factor_blas

# ------------------------------------------------------------
# Hand-coded matrix multiply
# ------------------------------------------------------------

factor: factor.o
	$(CC) $(CFLAGS) factor.o -o factor -lm

factor.o: factor.c
	$(CC) $(CFLAGS) -c factor.c -o factor.o

# ------------------------------------------------------------
# Apple Accelerate cblas_dgemm
# ------------------------------------------------------------

factor_blas: factor_blas.o
	$(ACCEL_CC) $(ACCEL_CFLAGS) factor_blas.o -o factor_blas \
	    -framework Accelerate -lm

factor_blas.o: factor_blas.c
	$(ACCEL_CC) $(ACCEL_CFLAGS) -c factor_blas.c -o factor_blas.o

# ------------------------------------------------------------

clean:
	rm -f factor factor_blas *.o
