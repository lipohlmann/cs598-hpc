#include <mpi.h>

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

  // initialize mesh
  Mesh mesh;
  if (world_rank == 0) {
    create_mesh(&mesh, Lx, Ly, Lz, nx, ny, nz);
  };

  // create GLL quadrature set
  QuadSet gll_quad = GaussLobattoLegendre(n_nodes_per_element);

  // create local mass matrix
  Matrix M = create_mass_matrix(&gll_quad);

  // create local stiffness matrix
  Matrix K = create_stiffness_matrix(&gll_quad);

  MPI_Finalize();

  return 0;
}
