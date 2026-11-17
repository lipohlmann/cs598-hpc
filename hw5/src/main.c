#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "poisson.h"
#include "serial_jacobi.h"
#include "vector.h"

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

  Matrix A = CreatePoissonMatrix(N, L);
  Vector b = CreateBoundaryVector(N, L, boundary_flag);

  // solve here...

  free_matrix(&A);
  free_vector(&b);

  return 0;
}
