#pragma once

#include <stddef.h>

/**
 * Convention: M = N - 1 interior points per side. Grid point (i, j) maps to
 * unknown index k = i * M + j, with i <-> x and j <-> y. Interior node i sits
 * at x = h * (i + 1), where h = L / N.
 */

/**
 * @brief Create a Poisson Matrix as a flattened 1D array (row-major).
 *
 * @param A 1D array with size (N-1)^2 x (N-1)^2
 * @param N N
 * @param L Length of square domain
 */
void CreatePoissonMatrix(double* A, const int N, const double L);

/**
 * @brief Create the l.h.s. of a Poisson problem where the forcing function is
 * only the boundary values
 *
 * @param b 1D array with size (N-1)^2
 * @param N N
 * @param L Length of square domain
 * @param boundary_type Index for boundary types given in EvaluateBoundary
 */
void CreateBoundaryVector(double* b, const int N, const double L,
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
 * @brief Computes the equivalent flat index given a row,col index pair. Assumes
 * 0-indexing!
 *
 * @param n_cols
 * @param row
 * @param col
 * @return size_t
 */
size_t FlatIndex(const size_t n_cols, const size_t row, const size_t col);