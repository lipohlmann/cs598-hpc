#include "matrix.h"

void random_spd_tridiag(tridiag_matrix* mat) {
  size_t n = mat->n;
  double* a = (double*)malloc(n * sizeof(double));
  double* b = (double*)malloc((n - 1) * sizeof(double));
  double* c = (double*)malloc((n - 1) * sizeof(double));

  // off-diagonals: c = b makes the matrix symmetric
  for (size_t i = 0; i < n - 1; i++) {
    b[i] = unbound_random_number();
    c[i] = b[i];
  }
  // diagonal: bigger than the sum of its row's off-diagonals, which together
  // with symmetry and a positive diagonal guarantees SPD
  for (size_t i = 0; i < n; i++) {
    a[i] = unbound_random_number();
    if (i > 0) a[i] += c[i - 1];
    if (i < n - 1) a[i] += b[i];
  }
  mat->diag = a;
  mat->subdiag = c;
  mat->superdiag = b;
}

double unbound_random_number() { return rand() / (RAND_MAX + 1.0); }
