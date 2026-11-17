#pragma once

#include <mpi.h>
#include <stddef.h>

/**
 * @brief Create the local Poisson operator and solve the system using a
 * parallel Jacobi algorithm.
 *
 * @param N N
 * @param L Length of the square domain
 * @param boundary_type 0,1,2 (see EvaluateBoundary in poisson.h/c)
 * @param tol Solver tolerance
 * @param max_iters Max iterations allowed
 * @param comm
 */
void MPIJacobi(const size_t N, const double L, const int boundary_type,
               const double tol, const int max_iters, MPI_Comm comm);
