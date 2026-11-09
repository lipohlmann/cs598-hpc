// Times a parallel tridiagonal solver on one problem size and appends the
// result as a line of a CSV file, creating the file (with a header) if needed.
// The rank count comes from mpirun, so a scaling study is a bash loop over
// (P, n) pairs that all point at the same CSV:
//
//   mpirun -np 16 build/scaling 1048576 strong.csv          # dd solver
//   mpirun -np 16 build/scaling 1048576 strong.csv thomas   # textbook solver

#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#include "matrix.h"
#include "thomas.h"
#include "tridiag.h"

// Flops serial Thomas needs per row: 3 to factor, 2 forward, 3 backward. GFLOPS
// is reported against this fixed count rather than what the parallel algorithm
// actually executes, so extra parallel work shows up as a lower rate and the
// numbers are comparable across rank counts.
static const double FLOPS_PER_ROW = 8.0;

static const double TOLERANCE = 1e-9;

// Returns 0 on success. The lock keeps jobs running at the same time from
// interleaving lines or both writing the header.
static int append_csv(const char* path, int P, int N, const char* solver,
                      int sweeps, double t_min, double t_mean, double t_max,
                      double gflops, double err) {
  FILE* fp = fopen(path, "a");
  if (!fp) return 1;
  flock(fileno(fp), LOCK_EX);

  fseek(fp, 0, SEEK_END);
  if (ftell(fp) == 0)
    fprintf(fp, "P,N,solver,sweeps,t_min,t_mean,t_max,gflops,max_err,status\n");
  fprintf(fp, "%d,%d,%s,%d,%.9e,%.9e,%.9e,%.6f,%.3e,%s\n", P, N, solver,
          sweeps, t_min, t_mean, t_max, gflops, err,
          err < TOLERANCE ? "PASS" : "FAIL");

  // closing flushes the buffer and then releases the lock
  return fclose(fp) != 0;
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int p, P;
  MPI_Comm_rank(MPI_COMM_WORLD, &p);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  if (argc < 3) {
    if (p == 0)
      fprintf(stderr, "usage: %s n results.csv [dd|thomas]\n", argv[0]);
    MPI_Finalize();
    return 1;
  }
  // dd is the domain-decomposition solver, thomas the textbook one
  const char* solver = argc > 3 ? argv[3] : "dd";
  bool use_dd = strcmp(solver, "dd") == 0;
  if (!use_dd && strcmp(solver, "thomas") != 0) {
    if (p == 0) fprintf(stderr, "unknown solver '%s'\n", solver);
    MPI_Finalize();
    return 1;
  }
  char* end;
  long n = strtol(argv[1], &end, 10);
  // the solvers take an int size. thomas needs a row on every rank, dd needs
  // a row in every block plus the P - 1 separators
  long n_min = use_dd ? 2L * P - 1 : P;
  if (*end != '\0' || n < n_min || n > INT_MAX) {
    if (p == 0)
      fprintf(stderr, "bad matrix size '%s': need %ld <= n <= %d\n", argv[1],
              n_min, INT_MAX);
    MPI_Finalize();
    return 1;
  }
  int N = (int)n;
  const char* csv_path = argv[2];

  // the rows of x this rank gets back from the solver: dd spreads them over
  // all ranks, thomas leaves the whole answer on rank 0
  int row_first = 0, row_count = p == 0 ? N : 0;
  if (use_dd) DomainDecompositionRows(p, P, N, &row_first, &row_count);

  int sweeps = 10;
  double t_min = INFINITY, t_max = 0, t_sum = 0, err = 0;

  // sweep -1 is a warm-up: it pays for MPI connection setup and first-touch
  // page faults, and is checked for correctness but not timed
  for (int k = -1; k < sweeps; k++) {
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

    // solve the system, timed. The barrier lines the ranks up so the
    // clock starts at the same moment everywhere
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();
    if (use_dd)
      DomainDecompositionParallel(p, P, N, matrix.subdiag, matrix.diag,
                                  matrix.superdiag, x, f);
    else
      ThomasAlgorithmParallel(p, P, N, matrix.subdiag, matrix.diag,
                              matrix.superdiag, x, f);
    double t_local = MPI_Wtime() - t0;

    // the solve is done when the slowest rank is
    double t;
    MPI_Reduce(&t_local, &t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // each rank checks the rows it got back
    for (int i = row_first; i < row_first + row_count; i++) {
      double e = fabs(x[i] - x_true[i]);
      if (e > err || isnan(e)) err = e;  // once NaN, err stays NaN
    }
    if (p == 0) {
      if (k >= 0) {
        t_sum += t;
        if (t < t_min) t_min = t;
        if (t > t_max) t_max = t;
      }
    }

    delete[] x;
    delete[] x_true;
    delete[] f;
    free(matrix.diag);
    free(matrix.subdiag);
    free(matrix.superdiag);
  }

  // worst error over all ranks. MPI_MAX can't be trusted with NaN, so a NaN
  // goes in as infinity, which still fails the check
  if (isnan(err)) err = INFINITY;
  double err_local = err;
  MPI_Reduce(&err_local, &err, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  int status = 0;
  if (p == 0) {
    // the best sweep is the one least disturbed by whatever else the node
    // was doing, so the rate comes from t_min
    double t_mean = t_sum / sweeps;
    double gflops = FLOPS_PER_ROW * N / t_min / 1e9;
    printf(
        "%-6s P=%-3d N=%-9d t_min=%.3e t_mean=%.3e gflops=%.3f max_err=%.3e "
        "%s\n",
        solver, P, N, t_min, t_mean, gflops, err,
        err < TOLERANCE ? "PASS" : "FAIL");
    if (append_csv(csv_path, P, N, solver, sweeps, t_min, t_mean, t_max, gflops,
                   err)) {
      perror(csv_path);
      status = 1;
    }
  }

  MPI_Finalize();
  return status;
}
