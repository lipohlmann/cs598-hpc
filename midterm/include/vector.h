#pragma once

#include <stdlib.h>

typedef struct {
  size_t size;
  double* data;
} Vector;

/**
 * @brief Create a vector object. All values in vector are initialized to 0.
 *
 * @param size
 * @return Vector
 */
Vector create_vector(const size_t size);

/**
 * @brief Free memory for members of vector
 *
 * @param vector
 */
void free_vector(Vector* vector);

/**
 * @brief Gets the value at a given index in the vector
 *
 * @param index
 * @param vector
 * @return double
 */
double vector_value_at(const size_t index, const Vector* vector);

/**
 * @brief Compute the dot product of two vectors. Returns NAN if sizes don't
 * match.
 *
 * @param v1
 * @param v2
 * @return double
 */
double dot(const Vector* v1, const Vector* v2);