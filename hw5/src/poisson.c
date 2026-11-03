#include "poisson.h"

void CreatePoissonMatrix(double* A, const int N, const double L) {
  const double h = L / (double)N;
  const double h_sqrd = h * h;
  const double off_diag = -1.0 / h_sqrd;  // check the sign...
  const double diag = 4.0 / h_sqrd;       // check the sign...
  const int matrix_size = N - 1;

  // check the following for bad indexing on the upper bound!
  for (int i = 1; i < matrix_size + 1; i++) {
    for (int j = 1; j < matrix_size + 1; j++) {
      A[FlatIndex(matrix_size, i, j)] = diag;
      A[FlatIndex(matrix_size, i - 1, j)] = off_diag;
      A[FlatIndex(matrix_size, i, j - 1)] = off_diag;
      A[FlatIndex(matrix_size, i + 1, j)] = off_diag;
      A[FlatIndex(matrix_size, i, j + 1)] = off_diag;
    }
  }
}

int FlatIndex(const int n_cols, const int row, const int col) {
  return row * n_cols + col;
}