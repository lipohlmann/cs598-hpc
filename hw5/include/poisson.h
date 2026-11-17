#pragma once

#include <stddef.h>

#include "matrix.h"
#include "vector.h"

/**
 * Convention: M = N - 1 interior points per side. Grid point (i, j) maps to
 * unknown index k = i * M + j, with i <-> x and j <-> y. Interior node i sits
 * at x = h * (i + 1), where h = L / N.
 */

/**
 * @brief Create the (dense) Poisson matrix. Caller owns the result and must
 * release it with free_matrix().
 *
 * @param N N
 * @param L Length of square domain
 * @return Matrix of size (N-1)^2 x (N-1)^2
 */
Matrix CreatePoissonMatrix(const size_t N, const double L);

/**
 * @brief Create the r.h.s. of a Poisson problem where the forcing function is
 * only the boundary values. Caller owns the result and must release it with
 * free_vector().
 *
 * @param N N
 * @param L Length of square domain
 * @param boundary_type Index for boundary types given in EvaluateBoundary
 * @return Vector of size (N-1)^2
 */
Vector CreateBoundaryVector(const size_t N, const double L,
                            const int boundary_type);

/**
 * @brief Evaluates the boundary condition.
 *
 * @param x x
 * @param y y
 * @param type Index for switch statement
 * @return double
 */
double EvaluateBoundary(const double x, const double y, const int type);

/**
 * @brief Maps a grid point (row, col) = (i, j) to its unknown index
 * k = row * n_cols + col. Used for the grid -> unknown mapping only; index
 * into a Matrix with flat_index(). Assumes 0-indexing!
 *
 * @param n_cols
 * @param row
 * @param col
 * @return size_t
 */
size_t FlatIndex(const size_t n_cols, const size_t row, const size_t col);
