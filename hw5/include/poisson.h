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
 * @brief Computes the equivalent flat index given a row,col index pair. Assumes
 * 0-indexing!
 *
 * @param n_cols
 * @param col
 * @param row
 * @return int
 */
int FlatIndex(const int n_cols, const int row, const int col);