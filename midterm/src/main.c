#include <mpi.h>

#include "broadcast.h"
#include "gather_scatter.h"
#include "gauss_lobatto_legendre.h"
#include "matrix.h"
#include "mesh.h"
#include "operator.h"
#include "rcb.h"

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
  Operator operator= { 0 };
  int* owner = NULL;  // rank that owns each element

  // build everything on rank 0
  if (world_rank == 0) {
    create_mesh(&mesh, Lx, Ly, Lz, nx, ny, nz);
    gll_quad = GaussLobattoLegendre(n_nodes_per_element);
    owner = rcb_partition(&mesh, world_size);
    M = create_mass_matrix(&gll_quad);
    K = create_stiffness_matrix(&gll_quad);
    operator.Kx = scale_by_constant(&K, 2.0 / Lx);
    operator.Ky = scale_by_constant(&K, 2.0 / Ly);
    operator.Kz = scale_by_constant(&K, 2.0 / Lz);
    operator.Mx = scale_by_constant(&M, Lx / 2.0);
    operator.My = scale_by_constant(&M, Ly / 2.0);
    operator.Mz = scale_by_constant(&M, Lz / 2.0);
  }
  operator.n = (size_t)n_nodes_per_element;

  // broadcast to all other ranks (collective)
  bcast_mesh(&mesh, 0, MPI_COMM_WORLD);
  bcast_partition(&owner, (nx) * (ny) * (nz), 0, MPI_COMM_WORLD);
  bcast_quad_set(&gll_quad, n_nodes_per_element, 0, MPI_COMM_WORLD);
  bcast_matrix(&M, 0, MPI_COMM_WORLD);
  bcast_matrix(&K, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Kx, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Ky, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Kz, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Mx, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.My, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Mz, 0, MPI_COMM_WORLD);

  // per-rank parts: element partition, mask, gather-scatter, inv_mult
  setup_operator_local(&operator, &mesh, owner, MPI_COMM_WORLD);

  free_operator(&operator);
  free(owner);
  free_mesh(&mesh);
  free_quad_set(&gll_quad);
  free_matrix(&M);
  free_matrix(&K);

  MPI_Finalize();

  return 0;
}
