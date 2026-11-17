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
  if (matrix->is_diagonal != 0) {
    if (i == j)
      return matrix->data[i];
    else
      return 0.0;
  }
  return matrix->data[flat_index(i, j, matrix)];
}

Vector mat_vec(const Matrix* A, const Vector* x) {
  Vector b = create_vector(x->size);

  if (A->is_diagonal == 1) {
    for (size_t i = 0; i < A->n_rows; i++) {
      b.data[i] = matrix_value_at(i, i, A) * vector_value_at(i, x);
    }
  } else {
    for (size_t i = 0; i < A->n_rows; i++) {
      for (size_t j = 0; j < A->n_cols; j++)
        b.data[i] += matrix_value_at(i, j, A) * vector_value_at(j, x);
    }
  }
  return b;
}

Matrix scale_by_constant(const Matrix* A, const double scale) {
  Matrix result = create_matrix(A->n_rows, A->n_cols, A->is_diagonal);
  for (size_t i = 0; i < A->n_rows * A->n_cols; i++)
    result.data[i] = A->data[i] * scale;
  return result;
}
