#include "matrix.h"

void random_spd_tridiag(tridiag_matrix* mat) {
  double* a = malloc(mat->n * sizeof(double));
  double* b = malloc((mat->n - 1) * sizeof(double));
  double* c = malloc((mat->n - 1) * sizeof(double));

  a[0] = unbound_random_number();
  for (size_t i = 1; i < mat->n + 1; i++) {
    c[i] = unbound_random_number();
    b[i] = unbound_random_number();
    a[i] = c[i] + b[i] + unbound_random_number();
  }
  mat->diag = a;
  mat->subdiag = c;
  mat->superdiag = b;
}

double unbound_random_number() { return rand() / (RAND_MAX + 1.0); }