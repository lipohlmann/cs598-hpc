#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

int main(int argc, char* argv[]) {
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

    MPI_Finalize();
  }

  return 0;
}
