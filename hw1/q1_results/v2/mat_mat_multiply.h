#pragma once

#include <stdlib.h>

/**
 * @brief Computes the matrix-matrix product C=AB
 *
 * @param A A matrix
 * @param A_rows Number of rows in A
 * @param A_cols Number of columns in A
 * @param B B matrix
 * @param B_rows Number of rows in B
 * @param B_cols Number of columns in B
 * @return double*
 */
double* MatMat(const double* restrict A, const size_t A_rows,
               const size_t A_cols, const double* restrict B,
               const size_t B_rows, const size_t B_cols);

/**
 * @brief Computes the i-j-th entry of the Matrix-Matrix product AB.
 *
 * @param A A matrix
 * @param A_cols Number of columns in A
 * @param B B matrix
 * @param B_cols Number of columns in B
 * @param row Row index (i)
 * @param col Column index (j)
 * @return double
 */
double MatMatEntry(const double* A, const size_t A_cols, const double* B,
                   const size_t B_cols, const size_t row, const size_t col);

/**
 * @brief Computes the 1D (flattened) index for an entry of a matrix. That is,
 * it converts a pair (i,j) to a single index k that indexes correctly into a 1D
 * (flattened) representation of the matrix. This is computed with:
 *
 * k = n_cols * row + col
 *
 * @param n_cols Number of columns in matrix
 * @param row Row index
 * @param col Column index
 * @return size_t
 */
size_t ComputeFlatIndex(const size_t n_cols, const size_t row,
                        const size_t col);
