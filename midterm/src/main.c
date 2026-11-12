#include <mpi.h>
#include <stdio.h>

#include "broadcast.h"
#include "cg.h"
#include "gather_scatter.h"
#include "gauss_lobatto_legendre.h"
#include "matrix.h"
#include "mesh.h"
#include "mms.h"
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

  int n_nodes_per_element = 12;  // must be at least 2!

  double cg_rel_tol = 1e-12;  // CG stops at ||r|| / ||b|| < cg_rel_tol
  int cg_max_iters = 1000;
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
    // map the reference element [-1,1] to an element of size dx, dy, dz
    operator.Kx = scale_by_constant(&K, 2.0 / mesh.dx);
    operator.Ky = scale_by_constant(&K, 2.0 / mesh.dy);
    operator.Kz = scale_by_constant(&K, 2.0 / mesh.dz);
    operator.Mx = scale_by_constant(&M, mesh.dx / 2.0);
    operator.My = scale_by_constant(&M, mesh.dy / 2.0);
    operator.Mz = scale_by_constant(&M, mesh.dz / 2.0);
  }
  operator.n =(size_t) n_nodes_per_element;

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
  setup_operator_local(&operator, & mesh, owner, MPI_COMM_WORLD);

  // manufactured solution: right-hand side, CG solve, error against exact
  MmsParams mms_params = {Lx, Ly, Lz};
  Vector b = create_rhs(&operator, & mesh, &gll_quad, mms_forcing, &mms_params);
  Vector u_exact =
      evaluate_field(&operator, & mesh, &gll_quad, mms_u_exact, &mms_params);
  Vector u = create_vector(b.size);  // zero initial guess

  int iters = conjugate_gradient(&u, &b, cg_rel_tol, cg_max_iters, &operator);

  double l2_error, linf_error;
  mms_error_norms(&operator, & u, &u_exact, &l2_error, &linf_error);
  if (world_rank == 0) {
    printf("ranks %d | CG iterations %d%s | L2 error %.6e | Linf error %.6e\n",
           world_size, iters, iters < 0 ? " (NOT CONVERGED)" : "", l2_error,
           linf_error);
  }

  free_vector(&b);
  free_vector(&u_exact);
  free_vector(&u);

  free_operator(&operator);
  free(owner);
  free_mesh(&mesh);
  free_quad_set(&gll_quad);
  free_matrix(&M);
  free_matrix(&K);

  MPI_Finalize();

  return 0;
}
