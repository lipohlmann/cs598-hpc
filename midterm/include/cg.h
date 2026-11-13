#pragma once

#include "vector.h"

/**
 * @brief Compute the operator application y = A x. Must overwrite y.
 *
 * For the SEM: local Kronecker apply per element, then gather-scatter
 * (gs_add), then the Dirichlet mask.
 *
 * @param y Result vector (overwritten)
 * @param x Input vector
 * @param ctx Opaque pointer to operator data
 */
void apply_A(Vector* y, const Vector* x, void* ctx);

/**
 * @brief Compute the global inner product of two vectors, counting each
 * shared node exactly once across elements and MPI ranks.
 *
 * @param a
 * @param b
 * @param ctx Opaque pointer to operator data
 * @return double
 */
double global_dot(const Vector* a, const Vector* b, void* ctx);

/**
 * @brief Solve A x = b with unpreconditioned conjugate gradient.
 *
 * @param x On entry, the initial guess; on exit, the solution
 * @param b Right-hand side (already assembled and masked)
 * @param rel_tol Stop when ||r|| / ||b|| < rel_tol
 * @param max_iters Iteration cap
 * @param ctx Opaque pointer passed through to apply_A and global_dot
 * @return Number of iterations taken, or -1 if not converged
 */
int conjugate_gradient(Vector* x, const Vector* b, const double rel_tol,
                       const int max_iters, void* ctx);