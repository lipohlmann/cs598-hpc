#pragma once

#include "gather_scatter.h"
#include "matrix.h"
#include "mesh.h"
#include "vector.h"

typedef struct {
  size_t n;           // GLL points per direction per element
  size_t n_elements;  // number of elements on this rank
  size_t* element_ids;  // global IDs of this rank's elements (ascending)

  Matrix Kx, Ky, Kz;  // scaled 1D stiffness matrices (dense)
  Matrix Mx, My, Mz;  // scaled 1D mass matrices (diagonal)

  Vector mask;      // 0 on Dirichlet nodes, 1 elsewhere
  Vector inv_mult;  // 1 / (number of local copies of each node)

  struct gs_data* gsh;  // gslib gather-scatter handle
  MPI_Comm mpi_comm;    // communicator for global reductions
} Operator;

/**
 * @brief Apply the local operator of one element,
 * \f$y = (M_z\otimes M_y\otimes K_x + M_z\otimes K_y\otimes M_x +
 * K_z\otimes M_y\otimes M_x + M_z\otimes M_y\otimes M_x)\,u\f$.
 *
 * Because the GLL mass matrices are diagonal, each Kronecker product only
 * needs one dense 1D sweep (the stiffness direction); the mass factors are
 * pointwise scalings. That is 3 n^4 multiply-adds per element instead of the
 * 12 n^4 of four generic SerialTripleKroneckerProduct() calls, with no
 * temporary vectors. Layout: x fastest, then y, then z (FlatIndex()).
 *
 * @param op Operator (n and the scaled 1D matrices)
 * @param u Element input, n^3 values
 * @param y Element output, n^3 values (overwritten; must not alias u)
 */
void local_operator_apply(const Operator* op, const double* restrict u,
                          double* restrict y);

/**
 * @brief Fill the per-rank parts of the operator: element list, n_elements,
 * mask, gslib handle, inv_mult and mpi_comm. The scaled matrices and op->n
 * must already be set on this rank. Collective.
 *
 * @param op Operator to fill
 * @param mesh Mesh (only the structured-grid description is used)
 * @param element_ids IDs of the elements owned by this rank (see
 * scatter_partition()). The operator takes ownership of the array.
 * @param n_local Number of entries in element_ids
 * @param comm Communicator
 */
void setup_operator_local(Operator* op, const Mesh* mesh, size_t* element_ids,
                          const size_t n_local, MPI_Comm comm);

/**
 * @brief Free everything owned by the operator.
 */
void free_operator(Operator* op);
