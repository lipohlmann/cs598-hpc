#pragma once

#include <mpi.h>

#include "matrix.h"
#include "mesh.h"
#include "quad_set.h"

/**
 * @brief Broadcast a mesh from root to all ranks. Collective: every rank must
 * call it. On non-root ranks the mesh is allocated and filled (release with
 * free_mesh); its prior contents are ignored.
 *
 * @param mesh Fully built on root; output on other ranks
 * @param root Rank that owns the data
 * @param comm Communicator
 */
void bcast_mesh(Mesh* mesh, int root, MPI_Comm comm);

/**
 * @brief Broadcast a quadrature set from root to all ranks. Collective. On
 * non-root ranks the set is allocated and filled (release with free_quad_set).
 *
 * @param quad_set Fully built on root; output on other ranks
 * @param n_points Number of points (must be known on every rank)
 * @param root Rank that owns the data
 * @param comm Communicator
 */
void bcast_quad_set(QuadSet* quad_set, int n_points, int root, MPI_Comm comm);

/**
 * @brief Broadcast a matrix from root to all ranks. Collective. Dimensions are
 * broadcast too; on non-root ranks the matrix is allocated and filled (release
 * with free_matrix).
 *
 * @param matrix Fully built on root; output on other ranks
 * @param root Rank that owns the data
 * @param comm Communicator
 */
void bcast_matrix(Matrix* matrix, int root, MPI_Comm comm);
