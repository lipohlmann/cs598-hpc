#ifndef MATRIX_H
#define MATRIX_H

#include <stdlib.h>

typedef struct tridiag_matrix {
  size_t n;
  double* diag;
  double* subdiag;
  double* superdiag;
} tridiag_matrix;

/**
 * @brief Creates a tridiagonal SPD matrix. Values on the diagonals are random.
 *
 * @param mat tridiag_matrix object. Must have n set.
 */
void random_spd_tridiag(tridiag_matrix* mat);

/**
 * @brief Computes and retuns an unbound random number between 0 and RAND_MAX.
 *
 * @return double
 */
double unbound_random_number();

#endif  // MATRIX_H