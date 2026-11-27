#pragma once

#include "vector.h"

typedef struct {
  Vector nodes;
  Vector weights;
} QuadSet;

/**
 * @brief Create a quadrature set with zero-initialized nodes and weights.
 *
 * @param n_points Number of points
 * @return QuadSet. Release with free_quad_set.
 */
QuadSet create_quad_set(const size_t n_points);

/**
 * @brief Free the memory held by a quadrature set.
 *
 * @param quad_set
 */
void free_quad_set(QuadSet* quad_set);
