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

  return y;
}

void SerialComputeW(Vector* w, const Matrix* C, const Vector* u) {
  for (size_t k = 0; k < w->size; k++) {
    for (size_t j = 0; j < w->size; j++) {
      for (size_t i = 0; i < w->size; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < w->size; p++)
          sum += matrix_value_at(i, p, C) *
                 vector_value_at(FlatIndex(p, j, k, u->size), u);

        w->data[FlatIndex(i, j, k, w->size)] = sum;
      }
    }
  }
}

void SerialComputeV(Vector* v, const Matrix* B, const Vector* w) {
  for (size_t k = 0; k < v->size; k++) {
    for (size_t j = 0; j < v->size; j++) {
      for (size_t i = 0; i < v->size; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < w->size; p++)
          sum += matrix_value_at(j, p, B) *
                 vector_value_at(FlatIndex(i, p, k, v->size), w);

        v->data[FlatIndex(i, j, k, w->size)] = sum;
      }
    }
  }
}

void SerialComputeY(Vector* y, const Matrix* A, const Vector* v) {
  for (size_t k = 0; k < y->size; k++) {
    for (size_t j = 0; j < y->size; j++) {
      for (size_t i = 0; i < y->size; i++) {
        double sum = 0.0;
        for (size_t p = 0; p < y->size; p++)
          sum += matrix_value_at(k, p, A) *
                 vector_value_at(FlatIndex(i, j, p, y->size), v);

        v->data[FlatIndex(i, j, k, y->size)] = sum;
      }
    }
  }
}

size_t FlatIndex(const size_t i, const size_t j, const size_t k,
                 const size_t n) {
  return i + j * n + k * n * n;
}