#include "kronecker.h"

Vector SerialTripleKroneckerProduct(const Vector* u, const Matrix* A,
                                    const Matrix* B, const Matrix* C) {
  const size_t vector_len = u->size;
  Vector w = create_vector(vector_len);
  SerialComputeW(&w, C, u);

  Vector v = create_vector(vector_len);
  SerialComputeV(&v, B, &w);

  Vector y = create_vector(vector_len);
  SerialComputeY(&y, A, &v);

  free_vector(&w);
  free_vector(&v);

  return y;
}

void SerialComputeW(Vector* w, const Matrix* C, const Vector* u) {
  const size_t n = C->n_rows;
  for (size_t k = 0; k < n; k++) {
    for (size_t j = 0; j < n; j++) {
      for (size_t i = 0; i < n; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < n; p++)
          sum += matrix_value_at(i, p, C) *
                 vector_value_at(FlatIndex(p, j, k, n), u);

        w->data[FlatIndex(i, j, k, n)] = sum;
      }
    }
  }
}

void SerialComputeV(Vector* v, const Matrix* B, const Vector* w) {
  const size_t n = B->n_rows;
  for (size_t k = 0; k < n; k++) {
    for (size_t j = 0; j < n; j++) {
      for (size_t i = 0; i < n; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < n; p++)
          sum += matrix_value_at(j, p, B) *
                 vector_value_at(FlatIndex(i, p, k, n), w);

        v->data[FlatIndex(i, j, k, n)] = sum;
      }
    }
  }
}

void SerialComputeY(Vector* y, const Matrix* A, const Vector* v) {
  const size_t n = A->n_rows;
  for (size_t k = 0; k < n; k++) {
    for (size_t j = 0; j < n; j++) {
      for (size_t i = 0; i < n; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < n; p++)
          sum += matrix_value_at(k, p, A) *
                 vector_value_at(FlatIndex(i, j, p, n), v);

        y->data[FlatIndex(i, j, k, n)] = sum;
      }
    }
  }
}

size_t FlatIndex(const size_t i, const size_t j, const size_t k,
                 const size_t n) {
  return i + j * n + k * n * n;
}

void SerialLocalOperator(const double* restrict u, double* restrict y,
                         const Matrix* Kx, const Matrix* Ky, const Matrix* Kz,
                         const Matrix* Mx, const Matrix* My, const Matrix* Mz) {
  const size_t n = Kx->n_rows;
  const size_t n2 = n * n;
  const double* restrict kx = Kx->data;  // dense, row-major n x n
  const double* restrict ky = Ky->data;
  const double* restrict kz = Kz->data;
  const double* restrict mx = Mx->data;  // diagonals, length n
  const double* restrict my = My->data;
  const double* restrict mz = Mz->data;

  // x-direction stiffness and the mass term, one x-line (fixed j, k) at a time:
  // y[i,j,k] = my[j] mz[k] (sum_p Kx[i,p] u[p,j,k] + mx[i] u[i,j,k])
  for (size_t k = 0; k < n; k++)
    for (size_t j = 0; j < n; j++) {
      const double* restrict u_line = u + j * n + k * n2;
      double* restrict y_line = y + j * n + k * n2;
      const double myz = my[j] * mz[k];
      for (size_t i = 0; i < n; i++) {
        double sum = mx[i] * u_line[i];
        for (size_t p = 0; p < n; p++) sum += kx[i * n + p] * u_line[p];
        y_line[i] = myz * sum;
      }
    }

  // y-direction stiffness: y[i,j,k] += mx[i] mz[k] sum_p Ky[j,p] u[i,p,k]
  // (innermost loop over i: unit stride)
  for (size_t k = 0; k < n; k++)
    for (size_t j = 0; j < n; j++) {
      double* restrict y_line = y + j * n + k * n2;
      for (size_t p = 0; p < n; p++) {
        const double a = mz[k] * ky[j * n + p];
        const double* restrict u_line = u + p * n + k * n2;
        for (size_t i = 0; i < n; i++) y_line[i] += a * mx[i] * u_line[i];
      }
    }

  // z-direction stiffness: y[i,j,k] += mx[i] my[j] sum_p Kz[k,p] u[i,j,p]
  for (size_t k = 0; k < n; k++)
    for (size_t p = 0; p < n; p++) {
      const double a = kz[k * n + p];
      for (size_t j = 0; j < n; j++) {
        const double b = a * my[j];
        const double* restrict u_line = u + j * n + p * n2;
        double* restrict y_line = y + j * n + k * n2;
        for (size_t i = 0; i < n; i++) y_line[i] += b * mx[i] * u_line[i];
      }
    }
}
