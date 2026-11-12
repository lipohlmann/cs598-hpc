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