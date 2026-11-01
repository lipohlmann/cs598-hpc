#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  MPI_Init(&argc, &argv);
  int p, P;
  MPI_Comm_rank(MPI_COMM_WORLD, &p);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  size_t ns[1] = {64};

  for (int i = 0; i < 1; i++) {
    tridiag_matrix matrix;
    matrix.n = ns[i];

    random_spd_tridiag(&matrix);

    /* TODO (Stage 0): build the FULL matrix with YOUR generator
     *   (same seed on every rank!) so you can run serial_thomas and
     *   compare. Arrays: a,b,c,f of length n.                          */

    /* TODO (Stage 0): x_serial = serial_thomas(...); print the residual
     *   on rank 0.                                                     */

    /* TODO (Stage 1): do the parallel algorithm with a plain
     *   for (p = 0; p < P; p++) loop, no MPI yet, then compare to Stage 0.
     *   Run this with `mpirun -np 1` so only one process exists.       */

    /* TODO (Stage 3): each rank does only its own block:
     *   partition -> local_sweep -> exchange/gather S pieces ->
     *   solve_schur -> back_substitute -> check.                       */

    MPI_Finalize();
  }

  return 0;
}
