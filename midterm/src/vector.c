#include "vector.h"

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