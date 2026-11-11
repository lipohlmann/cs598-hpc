#include "quad_set.h"

QuadSet create_quad_set(const size_t n_points) {
  QuadSet quad_set;
  quad_set.nodes = create_vector(n_points);
  quad_set.weights = create_vector(n_points);
  return quad_set;
}

void free_quad_set(QuadSet* quad_set) {
  free_vector(&quad_set->nodes);
  free_vector(&quad_set->weights);
}
