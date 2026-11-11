#include "vector.h"

#include <math.h>

Vector create_vector(const size_t size) {
  Vector vec;
  vec.data = calloc(size, sizeof(double));
  vec.size = size;
  return vec;
}

void free_vector(Vector* vector) {
  free(vector->data);
  vector->data = NULL;
}

double vector_value_at(const size_t index, const Vector* vector) {
  return vector->data[index];
}

double dot(const Vector* v1, const Vector* v2) {
  if (v1->size != v2->size) return NAN;

  double sum = 0.0;
  for (size_t i = 0; i < v1->size; i++) sum += v1->data[i] * v2->data[i];
  return sum;
}