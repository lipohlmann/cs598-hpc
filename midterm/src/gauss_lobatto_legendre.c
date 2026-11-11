#include "gauss_lobatto_legendre.h"

#include "legendre.h"

QuadSet GaussLobattoLegendre(const int n_points) {
  if (n_points < 2) return create_quad_set(0);
  QuadSet quad_set = create_quad_set((size_t)n_points);
  GaussLobattoLegendreNodes(&quad_set);
  GaussLobattoLegendreWeights(&quad_set);
  return quad_set;
}

void GaussLobattoLegendreNodes(QuadSet* gll_quad_set) {
  const int n_points = (int)gll_quad_set->nodes.size;
  if (n_points < 2) return;

  gll_quad_set->nodes.data[0] = -1.0;
  gll_quad_set->nodes.data[n_points - 1] = 1.0;
  Vector inner_nodes = AllLegendrePrimeRoots(n_points - 1);
  for (int k = 1; k < n_points - 1; k++)
    gll_quad_set->nodes.data[k] = inner_nodes.data[k - 1];
  free_vector(&inner_nodes);
}

void GaussLobattoLegendreWeights(QuadSet* gll_quad_set) {
  const int n_points = (int)gll_quad_set->nodes.size;
  if (n_points < 2) return;

  const double front_coeff = 2.0 / (double)(n_points - 1) / (double)(n_points);
  for (int k = 0; k < n_points; k++) {
    double legendre_weight =
        LegendrePolynomial(n_points - 1, gll_quad_set->nodes.data[k]);
    gll_quad_set->weights.data[k] =
        front_coeff / legendre_weight / legendre_weight;
  }
}
