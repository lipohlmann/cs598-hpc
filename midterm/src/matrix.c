#include "matrix.h"

Matrix create_matrix(const size_t n_rows, const size_t n_cols,
                     const int is_diagonal) {
  double* data = calloc(n_rows * n_cols, sizeof(double));
  Matrix matrix;
  matrix.n_rows = n_rows;
  matrix.n_cols = n_cols;
  matrix.data = data;
  matrix.is_diagonal = is_diagonal;
  return matrix;
}

void free_matrix(Matrix* matrix) {
  free(matrix->data);
  matrix->data = NULL;
}

size_t flat_index(const size_t row, const size_t col, const Matrix* matrix) {
  return row * matrix->n_cols + col;
}

double matrix_value_at(const size_t i, const size_t j, const Matrix* matrix) {
  if (matrix->is_diagonal == 0) {
    if (i == j)
      return matrix->data[i];
    else
      return 0.0;
  }
  return matrix->data[flat_index(i, j, matrix)];
}

Matrix create_mass_matrix(const QuadSet* gll_set) {
  Matrix mass_matrix = create_matrix(gll_set->nodes.size, 1, 1);
  for (size_t i = 0; i < gll_set->nodes.size; i++) {
    mass_matrix.data[i] = gll_set->weights.data[i];
  }
}