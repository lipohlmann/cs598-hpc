#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mat_mat_multiply.h"
#include "rand_matrix.h"

#define N_SAMPLES 63

// Global variable to store the start time
struct timespec start_time;

// Equivalent to tic()
void tic() { clock_gettime(CLOCK_MONOTONIC, &start_time); }

// Equivalent to toc()
double toc() {
  struct timespec end_time;
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                        (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
  printf("Elapsed time is %.6f seconds.\n", elapsed_time);
  return elapsed_time;
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  int Ns[N_SAMPLES];
  double gcs[N_SAMPLES];
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

    // Warm start
    {
      double* warm1 = MatMat(A1, N, N, B1, N, N);
      double* warm2 = MatMat(A2, N, N, B2, N, N);
      double* warm3 = MatMat(A3, N, N, B3, N, N);
      double* warm4 = MatMat(A4, N, N, B4, N, N);
      free(warm1);
      free(warm2);
      free(warm3);
      free(warm4);
    }

    tic();
    double* C1 = MatMat(A1, N, N, B1, N, N);
    double* C2 = MatMat(A2, N, N, B2, N, N);
    double* C3 = MatMat(A3, N, N, B3, N, N);
    double* C4 = MatMat(A4, N, N, B4, N, N);
    double* D1 = MatMat(C1, N, N, B1, N, N);
    double* D2 = MatMat(C2, N, N, B2, N, N);
    double* D3 = MatMat(C3, N, N, B3, N, N);
    double* D4 = MatMat(C4, N, N, B4, N, N);
    double elapsed_time = toc();

    double N3 = (double)N * (double)N * (double)N;
    double gflops = (8.0 * 2.0 * N3 / elapsed_time) / 1e9;

    Ns[k] = N;
    gcs[k] = gflops;
    printf("%d %.6f %.6f\n", Ns[k], elapsed_time, gcs[k]);
    k += 1;

    free(A1);
    free(B1);
    free(A2);
    free(B2);
    free(A3);
    free(B3);
    free(A4);
    free(B4);
    free(C1);
    free(C2);
    free(C3);
    free(C4);
    free(D1);
    free(D2);
    free(D3);
    free(D4);
  }

  FILE* csv_file = fopen("gflops.csv", "w");
  if (csv_file == NULL) {
    perror("fopen");
    return 1;
  }
  fprintf(csv_file, "N,GFLOPS_per_core\n");
  for (int i = 0; i < N_SAMPLES; i++) {
    fprintf(csv_file, "%d,%.6f\n", Ns[i], gcs[i]);
  }
  fclose(csv_file);

  return 0;
}
