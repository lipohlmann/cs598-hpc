#include "mat_mat_multiply.h"

#define TILE_ROWS 4
#define TILE_COLS 4

// Accumulates the TILE_ROWS x TILE_COLS block of C at (row0, col0) over the
// full K reduction in scalar registers, so each element of A is read once
// per k (reused across TILE_COLS columns) and each element of B is read
// once per k (reused across TILE_ROWS rows), instead of every C entry
// re-walking A and B from scratch with no reuse.
static void ComputeTile(const double* restrict A, size_t A_cols,
                         const double* restrict B, size_t B_cols,
                         double* restrict C, size_t row0, size_t col0,
                         size_t K) {
  const double* restrict a0 = A + (row0 + 0) * A_cols;
  const double* restrict a1 = A + (row0 + 1) * A_cols;
  const double* restrict a2 = A + (row0 + 2) * A_cols;
  const double* restrict a3 = A + (row0 + 3) * A_cols;

  double c00 = 0.0, c01 = 0.0, c02 = 0.0, c03 = 0.0;
  double c10 = 0.0, c11 = 0.0, c12 = 0.0, c13 = 0.0;
  double c20 = 0.0, c21 = 0.0, c22 = 0.0, c23 = 0.0;
  double c30 = 0.0, c31 = 0.0, c32 = 0.0, c33 = 0.0;

  for (size_t k = 0; k < K; k++) {
    const double* restrict b_row = B + k * B_cols + col0;
    const double b0 = b_row[0], b1 = b_row[1], b2 = b_row[2], b3 = b_row[3];

    double av = a0[k];
    c00 += av * b0;
    c01 += av * b1;
    c02 += av * b2;
    c03 += av * b3;

    av = a1[k];
    c10 += av * b0;
    c11 += av * b1;
    c12 += av * b2;
    c13 += av * b3;

    av = a2[k];
    c20 += av * b0;
    c21 += av * b1;
    c22 += av * b2;
    c23 += av * b3;

    av = a3[k];
    c30 += av * b0;
    c31 += av * b1;
    c32 += av * b2;
    c33 += av * b3;
  }

  double* restrict c0 = C + (row0 + 0) * B_cols + col0;
  double* restrict c1 = C + (row0 + 1) * B_cols + col0;
  double* restrict c2 = C + (row0 + 2) * B_cols + col0;
  double* restrict c3 = C + (row0 + 3) * B_cols + col0;
  c0[0] = c00; c0[1] = c01; c0[2] = c02; c0[3] = c03;
  c1[0] = c10; c1[1] = c11; c1[2] = c12; c1[3] = c13;
  c2[0] = c20; c2[1] = c21; c2[2] = c22; c2[3] = c23;
  c3[0] = c30; c3[1] = c31; c3[2] = c32; c3[3] = c33;
}

// Scalar fallback for row/column remainders that don't fill a full
// TILE_ROWS x TILE_COLS tile.
static void ComputeBlockScalar(const double* restrict A, size_t A_cols,
                                const double* restrict B, size_t B_cols,
                                double* restrict C, size_t row_start,
                                size_t row_end, size_t col_start,
                                size_t col_end, size_t K) {
  for (size_t row = row_start; row < row_end; row++) {
    const double* restrict a_row = A + row * A_cols;
    double* restrict c_row = C + row * B_cols;
    for (size_t col = col_start; col < col_end; col++) {
      double sum = 0.0;
      for (size_t k = 0; k < K; k++) {
        sum += a_row[k] * B[k * B_cols + col];
      }
      c_row[col] = sum;
    }
  }
}

double* MatMat(const double* restrict A, const size_t A_rows,
               const size_t A_cols, const double* restrict B,
               const size_t B_rows, const size_t B_cols) {
  (void)B_rows;
  double* restrict C = malloc(A_rows * B_cols * sizeof(double));

  const size_t row_tiled_end = (A_rows / TILE_ROWS) * TILE_ROWS;
  const size_t col_tiled_end = (B_cols / TILE_COLS) * TILE_COLS;

  for (size_t row = 0; row < row_tiled_end; row += TILE_ROWS) {
    for (size_t col = 0; col < col_tiled_end; col += TILE_COLS) {
      ComputeTile(A, A_cols, B, B_cols, C, row, col, A_cols);
    }
    if (col_tiled_end < B_cols) {
      ComputeBlockScalar(A, A_cols, B, B_cols, C, row, row + TILE_ROWS,
                          col_tiled_end, B_cols, A_cols);
    }
  }
  if (row_tiled_end < A_rows) {
    ComputeBlockScalar(A, A_cols, B, B_cols, C, row_tiled_end, A_rows, 0,
                        B_cols, A_cols);
  }

  return C;
}

double MatMatEntry(const double* A, const size_t A_cols, const double* B,
                   const size_t B_cols, const size_t row, const size_t col) {
  size_t K = A_cols;
  double sum = 0.0;
  for (size_t k = 0; k < K; k++) {
    size_t A_flat_index = ComputeFlatIndex(A_cols, row, k);
    size_t B_flat_index = ComputeFlatIndex(B_cols, k, col);
    sum += (A[A_flat_index] * B[B_flat_index]);
  }
  return sum;
}

size_t ComputeFlatIndex(const size_t n_cols, const size_t row,
                        const size_t col) {
  return n_cols * row + col;
}