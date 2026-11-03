#pragma once

/**
 * @brief Create a Poisson Matrix as a flattened 1D array.
 *
 * @param A 1D array with size (N+1) x (N+1)
 * @param N N
 * @param L Length of square domain
 */
void CreatePoissonMatrix(double* A, const int N, const double L);

/**
 * @brief Create the l.h.s. of a Poisson problem where the forcing function is
 * only the boundary values
 *
 * @param b 1D array with size (N+1)^2
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
 * @param col
 * @param row
 * @return int
 */
int FlatIndex(const int n_cols, const int row, const int col);