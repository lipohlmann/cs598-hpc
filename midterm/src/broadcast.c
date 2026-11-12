#include "broadcast.h"

#include <limits.h>
#include <stdio.h>

_Static_assert(sizeof(size_t) == sizeof(unsigned long long),
               "size_t must be 64-bit to be sent as unsigned long long");
_Static_assert(sizeof(Element) == 9 * sizeof(size_t),
               "Element must be 9 contiguous size_t values");

static int checked_count(const size_t n, MPI_Comm comm) {
  if (n > (size_t)INT_MAX) {
    fprintf(stderr, "broadcast: count %zu exceeds INT_MAX\n", n);
    MPI_Abort(comm, 1);
  }
  return (int)n;
}

void bcast_mesh(Mesh* mesh, int root, MPI_Comm comm) {
  int rank;
  MPI_Comm_rank(comm, &rank);

  // sizes and spacings
  unsigned long long dims[3] = {mesh->nx, mesh->ny, mesh->nz};
  double spacing[3] = {mesh->dx, mesh->dy, mesh->dz};
  MPI_Bcast(dims, 3, MPI_UNSIGNED_LONG_LONG, root, comm);
  MPI_Bcast(spacing, 3, MPI_DOUBLE, root, comm);

  const size_t nx = dims[0], ny = dims[1], nz = dims[2];
  const size_t n_nodes = nx * ny * nz;
  const size_t n_elements = (nx - 1) * (ny - 1) * (nz - 1);

  if (rank != root) {
    mesh->nx = nx;
    mesh->ny = ny;
    mesh->nz = nz;
    mesh->dx = spacing[0];
    mesh->dy = spacing[1];
    mesh->dz = spacing[2];
    mesh->xs = malloc(nx * sizeof(double));
    mesh->ys = malloc(ny * sizeof(double));
    mesh->zs = malloc(nz * sizeof(double));
    mesh->node_ids = malloc(n_nodes * sizeof(size_t));
    mesh->elements = malloc(n_elements * sizeof(Element));
  }

  MPI_Bcast(mesh->xs, checked_count(nx, comm), MPI_DOUBLE, root, comm);
  MPI_Bcast(mesh->ys, checked_count(ny, comm), MPI_DOUBLE, root, comm);
  MPI_Bcast(mesh->zs, checked_count(nz, comm), MPI_DOUBLE, root, comm);
  MPI_Bcast(mesh->node_ids, checked_count(n_nodes, comm),
            MPI_UNSIGNED_LONG_LONG, root, comm);

  // Element is 9 contiguous size_t values
  MPI_Datatype element_type;
  MPI_Type_contiguous(9, MPI_UNSIGNED_LONG_LONG, &element_type);
  MPI_Type_commit(&element_type);
  MPI_Bcast(mesh->elements, checked_count(n_elements, comm), element_type,
            root, comm);
  MPI_Type_free(&element_type);
}

void bcast_quad_set(QuadSet* quad_set, int n_points, int root, MPI_Comm comm) {
  int rank;
  MPI_Comm_rank(comm, &rank);

  if (rank != root) *quad_set = create_quad_set((size_t)n_points);

  MPI_Bcast(quad_set->nodes.data, n_points, MPI_DOUBLE, root, comm);
  MPI_Bcast(quad_set->weights.data, n_points, MPI_DOUBLE, root, comm);
}

void bcast_matrix(Matrix* matrix, int root, MPI_Comm comm) {
  int rank;
  MPI_Comm_rank(comm, &rank);

  unsigned long long dims[2] = {matrix->n_rows, matrix->n_cols};
  int is_diagonal = matrix->is_diagonal;
  MPI_Bcast(dims, 2, MPI_UNSIGNED_LONG_LONG, root, comm);
  MPI_Bcast(&is_diagonal, 1, MPI_INT, root, comm);

  if (rank != root) *matrix = create_matrix(dims[0], dims[1], is_diagonal);

  MPI_Bcast(matrix->data, checked_count(dims[0] * dims[1], comm), MPI_DOUBLE,
            root, comm);
}
