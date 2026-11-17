#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "poisson.h"
#include "serial_jacobi.h"
#include "vector.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  // ---------------------------------------------------------------------------
  // user-set stuff here! ------------------------------------------------------
  // ---------------------------------------------------------------------------
  size_t N = 11;
  double L = 1.0;
  int boundary_flag = 1;
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // HW5 (1): confirm k_max ~ d * kappa  (TODO: wrap the solve below in loops)
  // ---------------------------------------------------------------------------
  // for N in {8, 16, 32, 64}        (dense A is (N-1)^2 squared, so keep N
  // small)
  //   for d in {2, 4, 6, 8}         (tol_rel = 10^-d)
  //     - kappa has a closed form, no eigensolver needed. Eigenvalues of A:
  //         lambda_pq = (4/h^2) (sin^2(p*pi/(2N)) + sin^2(q*pi/(2N))),
  //         p, q = 1..N-1
  //       so kappa = lambda_max / lambda_min = cot^2(pi / (2N)) ~ 4 N^2 / pi^2.
  //       (h^2 cancels, so kappa does not depend on L.)
  //     - run SerialJacobi with the relative tolerance, record k
  //     - print/CSV: N, d, kappa, k, k / (d * kappa)
  //   Expectation: k / (d*kappa) is ~constant in N. Jacobi has
  //   rho = cos(pi/N), so k ~ d ln(10) / (1 - rho) = (ln(10)/2) d kappa
  //   ~ 1.15 d kappa. The in-class estimate holds up to that constant.
  //   Plot k vs N (log-log, slope 2) and k/(d*kappa) vs N (flat).
  //
  // "How did you determine the accuracy of your iterative solver?" Keep two
  // errors separate, and report both:
  //   1. SOLVER error:  ||x_k - x*|| / ||x*||, where x* solves A x = b
  //      exactly. Get x* from a very tight Jacobi run (~1e-14) or a direct
  //      solve. Expect it to track the relative residual to within ~kappa
  //      (small residual != small error for ill-conditioned A).
  //   2. DISCRETIZATION error: ||x* - u_e||_inf, with u_e = sin(2x)exp(-2y),
  //      evaluated at the interior nodes x = h*(i+1), y = h*(j+1). This is
  //      O(h^2); check that it drops 4x when N doubles. Iterating until the
  //      solver error is far below this just wastes iterations.
  // ---------------------------------------------------------------------------

  Matrix A = CreatePoissonMatrix(N, L);
  Vector b = CreateBoundaryVector(N, L, boundary_flag);

  // solve here...
  Vector x = SerialJacobi(&A, &b, 1e-8, 1000);

  free_matrix(&A);
  free_vector(&b);
  free_vector(&x);

  // ---------------------------------------------------------------------------
  // HW5 (2): MPI version -- TODO when you get there
  //  - MPI_Init(&argc, &argv) at the very top, MPI_Finalize() at the end
  //    (and drop the (void)argc/argv casts). Get rank and size from
  //    MPI_COMM_WORLD.
  //  - Only rank 0 prints. The dense A/b above are NOT used by the MPI solver
  //    (it is matrix-free), so only build them on rank 0 for the P=1
  //    cross-check.
  //  - Cross-check: with mpirun -np 1 the residual history must match
  //    SerialJacobi iteration for iteration. Jacobi is deterministic, so the
  //    iteration count and solution must also match for -np 4, 6, 16.
  //  - Time with MPI_Wtime around the solve; see mpi_jacobi.c for what to
  //    time separately.
  // ---------------------------------------------------------------------------

  // initialize MPI
  MPI_Init(&argc, &argv);

  // get number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  MPI_Finalize();

  return 0;
}
