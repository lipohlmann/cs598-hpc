#include "mat_mat_multiply.h"

double* MatMat(const double* A, const size_t A_rows, const size_t A_cols,
               const double* B, const size_t B_rows, const size_t B_cols) {}

void SetMatEntry(double* Mat, const size_t n_cols, const size_t row,
                 const size_t col, const double val) {
  size_t flat_index = ComputeFlatIndex(n_cols, row, col);
  Mat[flat_index] = val;
}

size_t ComputeFlatIndex(const size_t n_cols, const size_t row,
                        const size_t col) {
  return n_cols * row + col;
}