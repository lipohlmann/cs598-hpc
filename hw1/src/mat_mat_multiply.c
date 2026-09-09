#include "mat_mat_multiply.h"

double* MatMat(const double* A, const size_t A_rows, const size_t A_cols,
               const double* B, const size_t B_rows, const size_t B_cols) {
  double* result = malloc(A_rows * B_cols * sizeof(double));
  for (size_t i = 0; i < A_rows; i++) {
    for (size_t j = 0; j < B_cols; j++) {
      size_t results_flat_index = ComputeFlatIndex(B_cols, i, j);
      result[results_flat_index] = MatMatEntry(A, A_cols, B, B_cols, i, j);
    }
  }
  return result;
}

double MatMatEntry(const double* A, const size_t A_cols, const double* B,
                   const size_t B_cols, const size_t row, const size_t col) {
  size_t K = A_cols;
  double sum = 0.0;
  for (size_t k = 0; k < K; k++) {
    size_t A_flat_index = ComputeFlatIndex(A_cols, row, k);
    size_t B_flat_index = ComputeFlatIndex(B_cols, k, col);
    sum += (A[A_flat_index] * B[B_flat_index]);
  }
  return sum;
}

size_t ComputeFlatIndex(const size_t n_cols, const size_t row,
                        const size_t col) {
  return n_cols * row + col;
}