//
// Adapted from the Kingfisher project.
//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, kingfisher-project

#pragma once

#include "quad_set.h"

/**
 * @brief Create a 1D Gauss-Lobatto-Legendre quadrature set on the standard
 * interval \f$\xi\in [-1,1]\f$.
 *
 * @param n_points Number of points. Must be >=2; otherwise an empty set is
 * returned.
 * @return QuadSet. Release with free_quad_set.
 */
QuadSet GaussLobattoLegendre(const int n_points);

/**
 * @brief Fill the nodes of a quadrature set in place with
 * Gauss-Lobatto-Legendre nodes. The number of points is taken from the set's
 * nodes size, which must be >=2.
 *
 * @param gll_quad_set QuadSet to modify
 */
void GaussLobattoLegendreNodes(QuadSet* gll_quad_set);

/**
 * @brief Fill the weights of a quadrature set in place with
 * Gauss-Lobatto-Legendre weights. The nodes must already be set, and the
 * weights vector must be the same size as the nodes.
 *
 * @param gll_quad_set QuadSet to modify
 */
void GaussLobattoLegendreWeights(QuadSet* gll_quad_set);
