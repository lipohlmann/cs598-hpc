#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "mat_mat_multiply.h"
#include "rand_matrix.h"

// Global variable to store the start time
clock_t start_time;

// Equivalent to tic()
void tic() { start_time = clock(); }

// Equivalent to toc()
double toc() {
  clock_t end_time = clock();
  double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  printf("Elapsed time is %.6f seconds.\n", elapsed_time);
  return elapsed_time;
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  int Ns[62];
  double gcs[62];
  int k = 0;
  for (unsigned int N = 8; N <= 2000; N += 32) {
    double* A1 = RandRand(N, N);
    double* B1 = RandRand(N, N);

    double* A2 = RandRand(N, N);
    double* B2 = RandRand(N, N);

    double* A3 = RandRand(N, N);
    double* B3 = RandRand(N, N);

    double* A4 = RandRand(N, N);
    double* B4 = RandRand(N, N);

    tic();
    double* C1 = MatMat(A1, N, N, B1, N, N);
    double* C2 = MatMat(A2, N, N, B2, N, N);
    double* C3 = MatMat(A3, N, N, B3, N, N);
    double* C4 = MatMat(A4, N, N, B4, N, N);
    double elapsed_time = toc();

    size_t N3 = N * N * N;
    size_t g_flops = (8 * 2 * N3 * elapsed_time) / 1e9;

    unsigned int n_cores = 8;
    size_t g_flops_per_core = g_flops / n_cores;

    Ns[k] = N;
    gcs[k] = g_flops_per_core;
    k += 1;
  }

  return 0;
}
