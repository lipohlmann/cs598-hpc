#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"
#include "thomas.h"

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int p, P;
  MPI_Comm_rank(MPI_COMM_WORLD, &p);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  // small sizes matter: the solver forgets a bad start value over many rows
  int ns[4] = {16, 64, 1000, 65536};

  for (int k = 0; k < 4; k++) {
    int N = ns[k];
    tridiag_matrix matrix;
    matrix.n = N;

    // create matrix. Every rank runs this, and rand() starts from the same
    // default seed on each one, so all ranks build the same matrix
    random_spd_tridiag(&matrix);

    // initialize output vector
    double* x = new double[N];
    for (int i = 0; i < N; i++) x[i] = 0;

    // pick the answer, then build the forcing vector as f = A * x_true
    double* x_true = new double[N];
    for (int i = 0; i < N; i++) x_true[i] = unbound_random_number() - 0.5;

    double* f = new double[N];
    for (int i = 0; i < N; i++) {
      f[i] = matrix.diag[i] * x_true[i];
      if (i > 0) f[i] += matrix.subdiag[i - 1] * x_true[i - 1];
      if (i < N - 1) f[i] += matrix.superdiag[i] * x_true[i + 1];
    }

    // solve the system
    ThomasAlgorithmParallel(p, P, N, matrix.subdiag, matrix.diag,
                            matrix.superdiag, x, f);

    // x is only filled in on rank 0, so that's the only place to check it
    if (p == 0) {
      double err = 0;
      for (int i = 0; i < N; i++) {
        double e = fabs(x[i] - x_true[i]);
        if (!(e <= err)) err = e;  // not fmax: it silently ignores NaN
      }
      printf("P=%-3d N=%-7d max|x-x_true|=%.3e  %s\n", P, N, err,
             err < 1e-9 ? "PASS" : "FAIL");
    }

    delete[] x;
    delete[] x_true;
    delete[] f;
    free(matrix.diag);
    free(matrix.subdiag);
    free(matrix.superdiag);
  }

  MPI_Finalize();
  return 0;
}
