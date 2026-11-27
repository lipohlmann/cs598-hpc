#pragma once

#include "vector.h"

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