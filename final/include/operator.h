#pragma once

#include "gather_scatter.h"
#include "matrix.h"
#include "mesh.h"
#include "vector.h"

typedef struct {
  size_t n;             // GLL points per direction per element
  size_t n_elements;    // number of elements on this rank
  size_t* element_ids;  // global IDs of this rank's elements (ascending)

  Matrix Kx, Ky, Kz;  // scaled 1D stiffness matrices (dense)
  Matrix Mx, My, Mz;  // scaled 1D mass matrices (diagonal)
  Matrix C;           // 1D convection matrix (dense)

  Vector mask;      // 0 on Dirichlet nodes, 1 elsewhere
  Vector inv_mult;  // 1 / (number of local copies of each node)

  struct gs_data* gsh;  // gslib gather-scatter handle
  MPI_Comm mpi_comm;    // communicator for global reductions
} Operator;

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
