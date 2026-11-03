#include <stdio.h>
#include <stdlib.h>

#include "poisson.h"
#include "serial_jacobi.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // ---------------------------------------------------------------------------
  // user-set stuff here! ------------------------------------------------------
  // ---------------------------------------------------------------------------
  size_t N = 11;
  double L = 1.0;
  int boundary_flag = 1;
  // ---------------------------------------------------------------------------

  size_t M = N - 1;
  size_t n = M * M;

  double *A = malloc(n * n * sizeof(double));
  double *b = malloc(n * sizeof(double));

  CreatePoissonMatrix(A, N, L);
  CreateBoundaryVector(b, N, L, boundary_flag);

  // solve here...

  free(A);
  free(b);

  A = NULL;
  b = NULL;

  return 0;
}
