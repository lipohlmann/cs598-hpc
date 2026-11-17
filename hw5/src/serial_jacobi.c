#include "serial_jacobi.h"

#include <math.h>
#include <stdio.h>

Vector SerialJacobi(const Matrix* A, const Vector* b, const double tol,
                    const int max_iters) {
  const size_t n = b->size;
  Vector x_kp1 = create_vector(n);
  Vector x_k = create_vector(n);

  Vector residual = create_vector(n);

  double residual_norm = 1.0;  // dummy value

  int k = 0;
  while (k <= max_iters) {
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
      residual.data[i] = bi - sum - diag * x_k.data[i];
    }
    residual_norm = sqrt(dot(&residual, &residual));
    printf("k = %d, residual_norm = %.4e\n", k, residual_norm);
    if (residual_norm < tol) {
      free_vector(&x_k);
      free_vector(&residual);
      return x_kp1;
    }
    for (size_t i = 0; i < n; i++) x_k.data[i] = x_kp1.data[i];
  }
  printf("WARNING: SerialJacobi did not converge!\n");
  return x_kp1;
}
