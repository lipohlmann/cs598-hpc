#pragma once

#include <mpi.h>

#include "matrix.h"
#include "mesh.h"
#include "quad_set.h"

/**
 * @brief Broadcast the structured-grid description of a mesh from root to all
 * ranks: the vertex counts (nx, ny, nz), the spacings (dx, dy, dz) and the
 * vertex coordinates along each axis (xs, ys, zs), O(nx + ny + nz) values in
 * total. That is all the non-root ranks need: element IDs are mapped to grid
 * indices arithmetically. The global node_ids and elements arrays are NOT
 * sent; on non-root ranks they are set to NULL. Collective: every rank must
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

/**
 * @brief Send every rank the IDs of the elements it owns. Collective. Root
 * groups the element IDs by owner (rcb_group_by_part()) and scatters them, so
 * each rank receives only its own list (ascending IDs), not the global owner
 * array.
 *
 * @param owner Owner of each element (rcb_partition()); only read on root
 * @param n_elements Total number of elements; only read on root
 * @param local_ids Output: allocated array of this rank's element IDs
 * (release with free())
 * @param n_local Output: number of elements owned by this rank
 * @param root Rank that owns the partition
 * @param comm Communicator
 */
void scatter_partition(const int* owner, size_t n_elements,
                       size_t** local_ids, size_t* n_local, int root,
                       MPI_Comm comm);
