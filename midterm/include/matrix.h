#pragma once

#include <stdlib.h>

#include "quad_set.h"

typedef struct {
  size_t n_rows;
  size_t n_cols;
  double* data;
  int is_diagonal;  // 0=false, 1=true
} Matrix;

/**
 * @brief Create a matrix object. All data are initialized to 0.
 *
 * @param n_rows
 * @param n_cols
 * @return Matrix
 */
Matrix create_matrix(const size_t n_rows, const size_t n_cols,
                     const int is_diagonal);

/**
 * @brief Free matrix member memory.
 *
 * @param matrix
 */
void free_matrix(Matrix* matrix);

/**
 * @brief Compute the equivalent flat index given a row and column number.
 *
 * @param row
 * @param col
 * @param matrix
 * @return size_t
 */
size_t flat_index(const size_t row, const size_t col, const Matrix* matrix);

/**
 * @brief Gets the matrix value at a given row, column pair
 *
 * @param i Row number
 * @param j Column number
 * @param matrix
 * @return double
 */
double matrix_value_at(const size_t i, const size_t j, const Matrix* matrix);

/**
 * @brief Create a diagonal mass matrix using the GLL quadrature set
 *
 * @param gll_set
 * @return Matrix
 */
Matrix create_mass_matrix(const QuadSet* gll_set);
