#include "serial_jacobi.h"

#include <math.h>

Vector SerialJacobi(const Matrix* A, const Vector* b, const double tol) {
  const size_t n = b->size;
  Vector x_kp1 = create_vector(n);
  Vector x_k = create_vector(n);

  Vector error = create_vector(n);

  double error_norm = 1.0;  // dummy value

  int k = 0;
  while (k <= 100) {
    k++;
    for (size_t i = 0; i < n; i++) {
      double diag = A->data[flat_index(i, i, A)];
      double bi = b->data[i];

      double sum = 0.0;
      for (size_t j = 0; j < i; j++)
        sum += A->data[flat_index(i, j, A)] * x_k.data[j];

      for (size_t j = i + 1; j < n; j++)
        sum += A->data[flat_index(i, j, A)] * x_k.data[j];

      x_kp1.data[i] = 1.0 / diag * (bi - sum);
      error.data[i] = x_k.data[i] - x_kp1.data[i];
    }
    error_norm = sqrt(dot(&error, &error));
    if (error_norm < tol) {
      free_vector(&x_k);
      free_vector(&error);
      return x_kp1;
    }
    for (size_t i = 0; i < n; i++) x_k.data[i] = x_kp1.data[i];
  }
  return x_kp1;
}
