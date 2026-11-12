#include "broadcast.h"

#include <limits.h>
#include <stdio.h>

#include "rcb.h"

_Static_assert(sizeof(size_t) == sizeof(unsigned long long),
               "size_t must be 64-bit to be sent as unsigned long long");

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
    // the global vertex and element arrays are not needed off root
    mesh->node_ids = NULL;
    mesh->elements = NULL;
  }

  // vertex coordinates along each axis: O(nx + ny + nz) values
  MPI_Bcast(mesh->xs, checked_count(nx, comm), MPI_DOUBLE, root, comm);
  MPI_Bcast(mesh->ys, checked_count(ny, comm), MPI_DOUBLE, root, comm);
  MPI_Bcast(mesh->zs, checked_count(nz, comm), MPI_DOUBLE, root, comm);
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

void scatter_partition(const int* owner, size_t n_elements,
                       size_t** local_ids, size_t* n_local, int root,
                       MPI_Comm comm) {
  int rank, size;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  int* counts = NULL;
  int* displs = NULL;
  size_t* grouped = NULL;
  if (rank == root) {
    checked_count(n_elements, comm);  // displacements must fit in an int
    counts = malloc((size_t)size * sizeof(int));
    displs = malloc((size_t)size * sizeof(int));
    grouped = rcb_group_by_part(owner, n_elements, size, counts);
    displs[0] = 0;
    for (int r = 1; r < size; r++) displs[r] = displs[r - 1] + counts[r - 1];
  }

  int my_count = 0;
  MPI_Scatter(counts, 1, MPI_INT, &my_count, 1, MPI_INT, root, comm);

  *n_local = (size_t)my_count;
  *local_ids = malloc(((size_t)my_count + 1) * sizeof(size_t));
  MPI_Scatterv(grouped, counts, displs, MPI_UNSIGNED_LONG_LONG, *local_ids,
               my_count, MPI_UNSIGNED_LONG_LONG, root, comm);

  free(counts);
  free(displs);
  free(grouped);
}
