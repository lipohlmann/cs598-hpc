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

double ComputeLagrangeDerivative(const size_t i, const size_t j,
                                 const QuadSet* gll_quad_set) {
  const size_t n_points = gll_quad_set->weights.size;
  const double corner = (double)n_points * (double)(n_points - 1) / 4.0;
  if (i == 0 && j == 0)
    return -corner;
  else if (i == (n_points - 1) && j == (n_points - 1))
    return corner;
  else if (i != j) {
    double node_i = gll_quad_set->nodes.data[i];
    double node_j = gll_quad_set->nodes.data[j];
    double node_diff = node_i - node_j;
    double legendre_ratio = LegendrePolynomial(n_points - 1, node_i) /
                            LegendrePolynomial(n_points - 1, node_j);
    return legendre_ratio / node_diff;
  } else
    return 0.0;
}
