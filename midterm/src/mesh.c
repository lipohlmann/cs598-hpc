#include "mesh.h"

mesh create_mesh(const size_t n_rows, const size_t n_cols) {
  double* data = calloc(n_rows * n_cols, sizeof(double));
  mesh mesh;
  mesh.n_rows = n_rows;
  mesh.n_cols = n_cols;
  mesh.data = data;
  return mesh;
}

void free_mesh(mesh* mesh) {
  free(mesh->data);
  mesh->data = NULL;
}

size_t flat_index(const size_t row, const size_t col, const mesh* mesh) {
  return row * mesh->n_cols + col;
}