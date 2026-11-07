#pragma once

#include <stdlib.h>

typedef struct {
  size_t n_rows;
  size_t n_cols;
  double* data;
} mesh;

mesh create_mesh(const size_t n_rows, const size_t n_cols);

void free_mesh(mesh* mesh);

size_t flat_index(const size_t row, const size_t col, const mesh* mesh);
