#pragma once

#include "gather_scatter.h"
#include "matrix.h"
#include "mesh.h"
#include "vector.h"

typedef struct {
  size_t n;           // GLL points per direction per element
  size_t n_elements;  // number of elements on this rank

  Matrix Kx, Ky, Kz;  // scaled 1D stiffness matrices (dense)
  Matrix Mx, My, Mz;  // scaled 1D mass matrices (diagonal)

  Vector mask;      // 0 on Dirichlet nodes, 1 elsewhere
  Vector inv_mult;  // 1 / (number of local copies of each node)

  struct gs_data* gsh;  // gslib gather-scatter handle
  MPI_Comm mpi_comm;    // communicator for global reductions
} Operator;

/**
 * @brief Fill the per-rank parts of the operator: n_elements, mask, gslib
 * handle, inv_mult and mpi_comm. The scaled matrices and op->n must already
 * be set on this rank. Collective.
 */
void setup_operator_local(Operator* op, const Mesh* mesh, MPI_Comm comm);

/**
 * @brief Free everything owned by the operator.
 */
void free_operator(Operator* op);
