#include "poisson.h"

#include <math.h>

void CreatePoissonMatrix(double* A, const int N, const double L) {
  const double h = L / (double)N;
  const double h_sqrd = h * h;
  const double off_diag = -1.0 / h_sqrd;  // check the sign...
  const double diag = 4.0 / h_sqrd;       // check the sign...
  const int matrix_size = N - 1;

  for (int i = 0; i < matrix_size; i++) {
    for (int j = 0; j < matrix_size; j++) {
      A[FlatIndex(matrix_size, i, j)] = diag;
      if (i > 0) A[FlatIndex(matrix_size, i - 1, j)] = off_diag;
      if (j > 0) A[FlatIndex(matrix_size, i, j - 1)] = off_diag;
      if (i < matrix_size - 1) A[FlatIndex(matrix_size, i + 1, j)] = off_diag;
      if (j < matrix_size - 1) A[FlatIndex(matrix_size, i, j + 1)] = off_diag;
    }
  }
}

void CreateBoundaryVector(double* b, const int N, const double L,
                          const int boundary_type) {
  const double h = L / (double)N;
  const int matrix_size = N - 1;

  double x = 0.0;
  double y = 0.0;

  for (int i = 0; i < matrix_size; i++) {
    x = h * (double)i;
    b[FlatIndex(matrix_size, i, 0)] = EvaluateBoundary(x, 0, 1);
    b[FlatIndex(matrix_size, i, matrix_size - 1)] =
        EvaluateBoundary(x, L, boundary_type);
  }

  for (int j = 0; j < matrix_size; j++) {
    y = h * (double)j;
    b[FlatIndex(matrix_size, 0, j)] = EvaluateBoundary(0, y, 1);
    b[FlatIndex(matrix_size, matrix_size - 1, j)] =
        EvaluateBoundary(L, y, boundary_type);
  }
}

double EvaluateBoundary(const double x, const double y, const int type) {
  switch (type) {
    case 1:
      return 1.0;

    case 2:
      return sin(2.0 * x) * exp(-2.0 * y);

    default:
      return 0.0;
  }
}

int FlatIndex(const int n_cols, const int row, const int col) {
  return row * n_cols + col;
}