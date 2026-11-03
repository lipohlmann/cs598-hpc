#include "poisson.h"

#include <math.h>
#include <string.h>

void CreatePoissonMatrix(double* A, const int N, const double L) {
  const double h = L / (double)N;
  const double h_sqrd = h * h;
  const double off_diag = -1.0 / h_sqrd;
  const double diag = 4.0 / h_sqrd;
  const size_t M = (size_t)(N - 1);
  const size_t n = M * M;  // matrix is n x n

  memset(A, 0, n * n * sizeof(double));

  for (size_t i = 0; i < M; i++) {
    for (size_t j = 0; j < M; j++) {
      const size_t k = FlatIndex(M, i, j);
      A[FlatIndex(n, k, k)] = diag;
      if (i > 0) A[FlatIndex(n, k, k - M)] = off_diag;
      if (i < M - 1) A[FlatIndex(n, k, k + M)] = off_diag;
      if (j > 0) A[FlatIndex(n, k, k - 1)] = off_diag;
      if (j < M - 1) A[FlatIndex(n, k, k + 1)] = off_diag;
    }
  }
}

void CreateBoundaryVector(double* b, const int N, const double L,
                          const int boundary_type) {
  const double h = L / (double)N;
  const double inv_h_sqrd = 1.0 / (h * h);
  const size_t M = (size_t)(N - 1);

  memset(b, 0, M * M * sizeof(double));

  // y = 0 and y = L edges (j = 0 and j = M - 1), x varies with i
  for (size_t i = 0; i < M; i++) {
    const double x = h * (double)(i + 1);
    b[FlatIndex(M, i, 0)] +=
        inv_h_sqrd * EvaluateBoundary(x, 0.0, boundary_type);
    b[FlatIndex(M, i, M - 1)] +=
        inv_h_sqrd * EvaluateBoundary(x, L, boundary_type);
  }

  // x = 0 and x = L edges (i = 0 and i = M - 1), y varies with j
  for (size_t j = 0; j < M; j++) {
    const double y = h * (double)(j + 1);
    b[FlatIndex(M, 0, j)] +=
        inv_h_sqrd * EvaluateBoundary(0.0, y, boundary_type);
    b[FlatIndex(M, M - 1, j)] +=
        inv_h_sqrd * EvaluateBoundary(L, y, boundary_type);
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

size_t FlatIndex(const size_t n_cols, const size_t row, const size_t col) {
  return row * n_cols + col;
}
