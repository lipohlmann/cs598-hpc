#include "thomas.h"

#include <mpi.h>

#include <math.h>
#include <stdlib.h>

// d_k only depends on a ratio of entries of S, so S can be rescaled freely. This
// keeps the product of the R_k matrices from overflowing for large N.
static void RescaleS(double S[2][2]) {
  double m = fmax(fmax(fabs(S[0][0]), fabs(S[0][1])),
                       fmax(fabs(S[1][0]), fabs(S[1][1])));
  if (m > 1e100) {
    S[0][0] /= m;
    S[0][1] /= m;
    S[1][0] /= m;
    S[1][1] /= m;
  }
}

void ThomasAlgorithmParallel(int my_id, int P, int N, double* b, double* a,
                             double* c, double* x, double* q) {
  int rows_local, local_offset;
  double S[2][2], T[2][2], s1_tmp, s2_tmp;
  double *l, *d, *y;
  MPI_Status status;

  l = malloc(N * sizeof(double));
  d = malloc(N * sizeof(double));
  y = malloc(N * sizeof(double));

  // malloc doesn't initialize, need to set all values to 0
  for (int i = 0; i < N; i++) {
    l[i] = 0;
    d[i] = 0;
    y[i] = 0;
  }

  // initialize Schur complement to be the identity
  S[0][0] = 1.0;
  S[1][1] = 1.0;
  S[1][0] = 0.0;
  S[0][1] = 0.0;

  // find *my* rows
  rows_local = N / P;
  local_offset = my_id * rows_local;
  if (my_id == P - 1) rows_local += N % P;  // last rank takes the leftovers

  if (my_id == 0) {
    s1_tmp = a[local_offset] * S[0][0];
    S[1][0] = S[0][0];
    S[1][1] = S[0][1];
    S[0][1] = a[local_offset] * S[0][1];
    S[0][0] = s1_tmp;

    for (int i = 1; i < rows_local; i++) {
      s1_tmp = a[i + local_offset] * S[0][0] -
               b[i + local_offset - 1] * c[i + local_offset - 1] * S[1][0];
      s2_tmp = a[i + local_offset] * S[0][1] -
               b[i + local_offset - 1] * c[i + local_offset - 1] * S[1][1];
      S[1][0] = S[0][0];
      S[1][1] = S[0][1];
      S[0][0] = s1_tmp;
      S[0][1] = s2_tmp;
      RescaleS(S);
    }
  } else {
    for (int i = 0; i < rows_local; i++) {
      s1_tmp = a[i + local_offset] * S[0][0] -
               b[i + local_offset - 1] * c[i + local_offset - 1] * S[1][0];
      s2_tmp = a[i + local_offset] * S[0][1] -
               b[i + local_offset - 1] * c[i + local_offset - 1] * S[1][1];
      S[1][0] = S[0][0];
      S[1][1] = S[0][1];
      S[0][0] = s1_tmp;
      S[0][1] = s2_tmp;
      RescaleS(S);
    }
  }

  // full-recursive doubling algorithm for distribution
  for (int i = 0; i < log2(P); i++) {
    if (my_id + pow(2, i) < P)
      MPI_Send(S, 4, MPI_DOUBLE, (int)(my_id + pow(2, i)), 0,
               MPI_COMM_WORLD);
    if (my_id - pow(2, i) >= 0) {
      MPI_Recv(T, 4, MPI_DOUBLE, (int)(my_id - pow(2, i)), 0,
               MPI_COMM_WORLD, &status);
      s1_tmp = S[0][0] * T[0][0] + S[0][1] * T[1][0];
      S[0][1] = S[0][0] * T[0][1] + S[0][1] * T[1][1];
      S[0][0] = s1_tmp;
      s1_tmp = S[1][0] * T[0][0] + S[1][1] * T[1][0];
      S[1][1] = S[1][0] * T[0][1] + S[1][1] * T[1][1];
      S[1][0] = s1_tmp;
      RescaleS(S);
    }
  }

  // calculate last d_k first so it can be distributed, then distribute
  d[local_offset + rows_local - 1] = (S[0][0] + S[0][1]) / (S[1][0] + S[1][1]);
  if (my_id == 0) {
    // first processor can't receive anything
    if (P > 1)
      MPI_Send(&d[local_offset + rows_local - 1], 1, MPI_DOUBLE, 1, 0,
               MPI_COMM_WORLD);
  } else {
    // middle and end processors receive information
    MPI_Recv(&d[local_offset - 1], 1, MPI_DOUBLE, my_id - 1, 0, MPI_COMM_WORLD,
             &status);
    if (my_id != P - 1) {
      // middle processors send
      MPI_Send(&d[local_offset + rows_local - 1], 1, MPI_DOUBLE, my_id + 1, 0,
               MPI_COMM_WORLD);
    }
  }

  // compute (in parallel) the local k_k and l_k values
  if (my_id == 0) {
    l[0] = 0;
    d[0] = a[0];
    for (int i = 1; i < rows_local - 1; i++) {
      l[local_offset + i] = b[local_offset + i - 1] / d[local_offset + i - 1];
      d[local_offset + i] =
          a[local_offset + i] - l[local_offset + i] * c[local_offset + i - 1];
    }
    if (rows_local > 1)
      l[local_offset + rows_local - 1] =
          b[local_offset + rows_local - 2] / d[local_offset + rows_local - 2];
  } else {
    for (int i = 0; i < rows_local - 1; i++) {
      l[local_offset + i] = b[local_offset + i - 1] / d[local_offset + i - 1];
      d[local_offset + i] =
          a[local_offset + i] - l[local_offset + i] * c[local_offset + i - 1];
    }
    l[local_offset + rows_local - 1] =
        b[local_offset + rows_local - 2] / d[local_offset + rows_local - 2];
  }

  if (my_id > 0) d[local_offset - 1] = 0;

  // distribute to processes
  double* tmp = malloc(N * sizeof(double));
  for (int i = 0; i < N; i++) tmp[i] = d[i];
  MPI_Allreduce(tmp, d, N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  for (int i = 0; i < N; i++) tmp[i] = l[i];
  MPI_Allreduce(tmp, l, N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  free(tmp);

  if (my_id == 0) {
    // forward substitution
    y[0] = q[0];
    for (int i = 1; i < N; i++) y[i] = q[i] - l[i] * y[i - 1];

    // backward substitution
    x[N - 1] = y[N - 1] / d[N - 1];
    for (int i = N - 2; i >= 0; i--) x[i] = (y[i] - c[i] * x[i + 1]) / d[i];
  }

  // cleanup
  free(l);
  free(y);
  free(d);
  return;
}