#pragma once

#include <stdlib.h>

typedef struct {
  size_t size;
  double* data;
} Vector;

Vector create_vector(const size_t size);

void free_vector(Vector* vector);
