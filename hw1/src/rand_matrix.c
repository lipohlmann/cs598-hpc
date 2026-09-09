#include "rand_matrix.h"

#include <stdlib.h>

#include "mat_mat_multiply.h"

double* RandRand(const size_t n_rows, const size_t n_cols) {
  double* results = malloc(n_rows * n_cols * sizeof(double));

  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_cols; j++) {
      double random_val = rand();
      size_t flat_index = ComputeFlatIndex(n_cols, i, j);
      results[flat_index] = random_val;
    }
  }
  return results;
}