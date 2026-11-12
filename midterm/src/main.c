#include <mpi.h>

#include "broadcast.h"
#include "gauss_lobatto_legendre.h"
#include "matrix.h"
#include "mesh.h"

int main(int argc, char const* argv[]) {
  // USER PARAMETERS
  double Lx = 1.0;
  double Ly = 1.0;
  double Lz = 1.0;

  size_t nx = 4;
  size_t ny = 4;
  size_t nz = 4;

  int n_nodes_per_element = 4;  // must be at least 2!
  // --------------------------------

  // initialize MPI
  MPI_Init(&argc, &argv);

  // get number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  Mesh mesh = {0};
  QuadSet gll_quad = {0};
  Matrix M = {0};
  Matrix K = {0};

  // build everything on rank 0
  if (world_rank == 0) {
    create_mesh(&mesh, Lx, Ly, Lz, nx, ny, nz);
    gll_quad = GaussLobattoLegendre(n_nodes_per_element);
    M = create_mass_matrix(&gll_quad);
    K = create_stiffness_matrix(&gll_quad);
  }

  // broadcast to all other ranks (collective)
  bcast_mesh(&mesh, 0, MPI_COMM_WORLD);
  bcast_quad_set(&gll_quad, n_nodes_per_element, 0, MPI_COMM_WORLD);
  bcast_matrix(&M, 0, MPI_COMM_WORLD);
  bcast_matrix(&K, 0, MPI_COMM_WORLD);

  free_mesh(&mesh);
  free_quad_set(&gll_quad);
  free_matrix(&M);
  free_matrix(&K);

  MPI_Finalize();

  return 0;
}
