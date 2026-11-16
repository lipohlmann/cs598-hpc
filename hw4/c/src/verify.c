// Checks that a parallel tridiagonal solver really solves A x = f on one
// problem size, and appends the residual as a line of a CSV file, creating the
// file (with a header) if needed. Nothing is timed. The rank count comes from
// mpirun, so a verification sweep is a bash loop over (P, n) pairs that all
// point at the same CSV:
//
//   mpirun -np 16 build/verify 65536 verify.csv          # dd solver
//   mpirun -np 16 build/verify 65536 verify.csv thomas   # textbook solver
//
// The residual r = f - A x is computed from the x the solver returned, so it
// does not depend on knowing the answer. The error against the x_true that f
// was built from is recorded next to it as a second, independent check.

// fileno and flock are POSIX, not plain C11
#define _DEFAULT_SOURCE

#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#include "matrix.h"
#include "thomas.h"
#include "tridiag.h"

// Pass mark for the relative residual ||f - A x||_2 / ||f||_2. A stable solve
// of these diagonally dominant systems lands near 1e-16; a wrong one is O(1).
static const double TOLERANCE = 1e-10;

// Each trial draws a new matrix and right-hand side; the worst one is reported.
static const int TRIALS = 3;

// max that lets NaN win, unlike fmax, so a broken solve can't hide
static double worst(double a, double b) { return (b > a || isnan(b)) ? b : a; }

// Returns 0 on success. The lock keeps jobs running at the same time from
// interleaving lines or both writing the header.
static int append_csv(const char* path, int P, int N, const char* solver,
                      double res_inf, double res_rel, double err, bool pass) {
  FILE* fp = fopen(path, "a");
  if (!fp) return 1;
  flock(fileno(fp), LOCK_EX);

  fseek(fp, 0, SEEK_END);
  if (ftell(fp) == 0)
    fprintf(fp, "P,N,solver,trials,res_inf,res_rel,max_err,status\n");
  fprintf(fp, "%d,%d,%s,%d,%.3e,%.3e,%.3e,%s\n", P, N, solver, TRIALS, res_inf,
          res_rel, err, pass ? "PASS" : "FAIL");

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

  // dd leaves each rank holding only its own rows of x, so rank 0 needs to
  // know where everyone's rows go to put the full vector back together.
  // thomas already leaves the whole answer on rank 0
  int row_first = 0, row_count = 0;
  int* counts = NULL;
  int* firsts = NULL;
  if (use_dd) {
    DomainDecompositionRows(p, P, N, &row_first, &row_count);
    if (p == 0) {
      counts = malloc(P * sizeof(int));
      firsts = malloc(P * sizeof(int));
      for (int r = 0; r < P; r++)
        DomainDecompositionRows(r, P, N, &firsts[r], &counts[r]);
    }
  }

  double res_inf = 0, res_rel = 0, err = 0;

  for (int k = 0; k < TRIALS; k++) {
    tridiag_matrix matrix;
    matrix.n = N;

    // create matrix. Every rank runs this, and rand() starts from the same
    // default seed on each one, so all ranks build the same matrix
    random_spd_tridiag(&matrix);

    // initialize output vector. NaN rather than 0, so a row the solver never
    // wrote poisons the residual instead of passing for a small number
    double* x = malloc(N * sizeof(double));
    for (int i = 0; i < N; i++) x[i] = NAN;

    // pick the answer, then build the forcing vector as f = A * x_true
    double* x_true = malloc(N * sizeof(double));
    for (int i = 0; i < N; i++) x_true[i] = unbound_random_number() - 0.5;

    double* f = malloc(N * sizeof(double));
    for (int i = 0; i < N; i++) {
      f[i] = matrix.diag[i] * x_true[i];
      if (i > 0) f[i] += matrix.subdiag[i - 1] * x_true[i - 1];
      if (i < N - 1) f[i] += matrix.superdiag[i] * x_true[i + 1];
    }

    // solve the system
    if (use_dd)
      DomainDecompositionParallel(p, P, N, matrix.subdiag, matrix.diag,
                                  matrix.superdiag, x, f);
    else
      ThomasAlgorithmParallel(p, P, N, matrix.subdiag, matrix.diag,
                              matrix.superdiag, x, f);

    // collect the full solution on rank 0. Rank 0's own rows start at 0, so
    // they are already where they belong
    if (use_dd)
      MPI_Gatherv(p == 0 ? MPI_IN_PLACE : x + row_first, row_count, MPI_DOUBLE,
                  x, counts, firsts, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (p == 0) {
      // r = f - A x, row by row
      double r_max = 0, r_sq = 0, f_sq = 0, e_max = 0;
      for (int i = 0; i < N; i++) {
        double r = f[i] - matrix.diag[i] * x[i];
        if (i > 0) r -= matrix.subdiag[i - 1] * x[i - 1];
        if (i < N - 1) r -= matrix.superdiag[i] * x[i + 1];
        r_max = worst(r_max, fabs(r));
        r_sq += r * r;
        f_sq += f[i] * f[i];
        e_max = worst(e_max, fabs(x[i] - x_true[i]));
      }
      res_inf = worst(res_inf, r_max);
      res_rel = worst(res_rel, sqrt(r_sq / f_sq));
      err = worst(err, e_max);
    }

    free(x);
    free(x_true);
    free(f);
    free(matrix.diag);
    free(matrix.subdiag);
    free(matrix.superdiag);
  }
  free(counts);
  free(firsts);

  int status = 0;
  if (p == 0) {
    // NaN compares false, so it fails
    bool pass = res_rel < TOLERANCE;
    printf("%-6s P=%-3d N=%-9d res_inf=%.3e res_rel=%.3e max_err=%.3e %s\n",
           solver, P, N, res_inf, res_rel, err, pass ? "PASS" : "FAIL");
    if (append_csv(csv_path, P, N, solver, res_inf, res_rel, err, pass)) {
      perror(csv_path);
      status = 1;
    }
  }

  MPI_Finalize();
  return status;
}
