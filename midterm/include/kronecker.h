#pragma once
#include <stdlib.h>

#include "matrix.h"
#include "vector.h"

/**
 * @brief Computes the triple Kronecker product (A x B x C)u=y,
 * where A, B, C are square matrices of size n by n.
 *
 * @param x
 * @param A
 * @param B
 * @param C
 * @param n
 */
Vector SerialTripleKroneckerProduct(const Vector* u, const Matrix* A,
                                    const Matrix* B, const Matrix* C);

/**
 * @brief Computes the quantity w = (I x I x C)u
 *
 * @param w
 * @param C
 * @param u
 */
void SerialComputeW(Vector* w, const Matrix* C, const Vector* u);

/**
 * @brief Computes the quantity v = (I x B x I)w
 *
 * @param v
 * @param B
 * @param w
 */
void SerialComputeV(Vector* v, const Matrix* B, const Vector* w);

/**
 * @brief Computes the quantity y = (A x I x I)v
 *
 * @param y
 * @param A
 * @param v
 */
void SerialComputeY(Vector* y, const Matrix* A, const Vector* v);

/**
 * @brief Compute the equivalent flat index for a 3-value index into a vector.
 *
 * @param i fast index
 * @param j middle index
 * @param k slow index
 * @param n number of values per direction
 * @return size_t
 */
size_t FlatIndex(const size_t i, const size_t j, const size_t k,
                 const size_t n);

/**
 * @brief Apply the local operator of one spectral element,
 * \f$y = (M_z\otimes M_y\otimes K_x + M_z\otimes K_y\otimes M_x +
 * K_z\otimes M_y\otimes M_x + M_z\otimes M_y\otimes M_x)\,u\f$.
 *
 * The mass matrices Mx, My, Mz must be diagonal (GLL mass matrices, stored as
 * their n diagonal entries). Then each Kronecker product only needs one dense
 * 1D sweep (the stiffness direction) and the mass factors are pointwise
 * scalings: 3 n^4 multiply-adds instead of the 12 n^4 of four
 * SerialTripleKroneckerProduct() calls, with no temporary vectors and unit
 * stride in every innermost loop. Layout: x fastest, then y, then z
 * (FlatIndex()).
 *
 * @param u Input, n^3 values
 * @param y Output, n^3 values (overwritten; must not alias u)
 * @param Kx Dense n x n stiffness matrix in x
 * @param Ky Dense n x n stiffness matrix in y
 * @param Kz Dense n x n stiffness matrix in z
 * @param Mx Diagonal mass matrix in x
 * @param My Diagonal mass matrix in y
 * @param Mz Diagonal mass matrix in z
 */
void SerialLocalOperator(const double* restrict u, double* restrict y,
                         const Matrix* Kx, const Matrix* Ky, const Matrix* Kz,
                         const Matrix* Mx, const Matrix* My, const Matrix* Mz);
