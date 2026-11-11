#include "matrix.h"

Matrix create_matrix(const size_t n_rows, const size_t n_cols) {
  double* data = calloc(n_rows * n_cols, sizeof(double));
  Matrix matrix;
  matrix.n_rows = n_rows;
  matrix.n_cols = n_cols;
  matrix.data = data;
  return matrix;
}

void free_matrix(Matrix* matrix) {
  free(matrix->data);
  matrix->data = NULL;
}

size_t flat_index(const size_t row, const size_t col, const Matrix* matrix) {
  return row * matrix->n_cols + col;
}