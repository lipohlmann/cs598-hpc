#pragma once

#include <stdlib.h>

typedef struct {
  size_t n_rows;
  size_t n_cols;
  double* data;
} Matrix;

Matrix create_matrix(const size_t n_rows, const size_t n_cols);

void free_matrix(Matrix* matrix);

size_t flat_index(const size_t row, const size_t col, const Matrix* matrix);

double value_at(const size_t i, const size_t j, const Matrix* matrix);
