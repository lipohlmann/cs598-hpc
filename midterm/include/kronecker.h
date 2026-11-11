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