#include "mat_mat_multiply.h"

// Register-resident accumulator tile shape (rows x cols). TILE_ROWS *
// TILE_COLS doubles must live in vector registers for the whole K
// reduction (see ComputeTile), so this is sized for 256-bit AVX2 vectors
// (16 YMM registers x 4 doubles each): 6x8 = 48 accumulator doubles (12
// YMM registers), leaving headroom for the per-k A/B temporaries.
#if defined(__AVX2__) && defined(__FMA__)
#define TILE_ROWS 6
#define TILE_COLS 8
#elif defined(__AVX__)
#define TILE_ROWS 4
#define TILE_COLS 4
#else
#define TILE_ROWS 2
#define TILE_COLS 4
#endif

// Accumulates a TILE_ROWS x TILE_COLS tile of C over the reduction depth
// [k_start, k_end). The accumulator stays in local arrays for the whole
// reduction (the compiler keeps these in registers), so C is only read and
// written once per call instead of once per k.
static void ComputeTile(const double* restrict A, size_t inner_dim,
                         const double* restrict B, size_t num_cols,
                         double* restrict C, size_t row_start, size_t k_start,
                         size_t k_end, size_t col_start) {
  double* restrict c_row[TILE_ROWS];
  const double* restrict a_row[TILE_ROWS];
  for (size_t r = 0; r < TILE_ROWS; r++) {
    c_row[r] = C + (row_start + r) * num_cols + col_start;
    a_row[r] = A + (row_start + r) * inner_dim;
  }

  double acc[TILE_ROWS][TILE_COLS];
  for (size_t r = 0; r < TILE_ROWS; r++)
    for (size_t c = 0; c < TILE_COLS; c++) acc[r][c] = c_row[r][c];

  for (size_t k = k_start; k < k_end; k++) {
    double a_val[TILE_ROWS];
    for (size_t r = 0; r < TILE_ROWS; r++) a_val[r] = a_row[r][k];
    const double* restrict b_row = B + k * num_cols + col_start;

    for (size_t r = 0; r < TILE_ROWS; r++) {
#pragma omp simd
      for (size_t c = 0; c < TILE_COLS; c++) {
        acc[r][c] += a_val[r] * b_row[c];
      }
    }
  }

  for (size_t r = 0; r < TILE_ROWS; r++)
    for (size_t c = 0; c < TILE_COLS; c++) c_row[r][c] = acc[r][c];
}

// Scalar fallback for the row/column remainder that doesn't fill a full
// TILE_ROWS x TILE_COLS tile.
static void ComputeTileScalar(const double* restrict A, size_t inner_dim,
                               const double* restrict B, size_t num_cols,
                               double* restrict C, size_t row_start,
                               size_t row_end, size_t k_start, size_t k_end,
                               size_t col_start, size_t col_end) {
  for (size_t row = row_start; row < row_end; row++) {
    double* restrict c_row = C + row * num_cols;
    const double* restrict a_row = A + row * inner_dim;
    for (size_t k = k_start; k < k_end; k++) {
      const double a_val = a_row[k];
      const double* restrict b_row = B + k * num_cols;
#pragma omp simd
      for (size_t col = col_start; col < col_end; col++) {
        c_row[col] += a_val * b_row[col];
      }
    }
  }
}

// Multiplies rows [row_start, row_end) of A by B's [k_start, k_end) x
// [0, num_cols) panel into the matching rows of C, tiling into
// TILE_ROWS x TILE_COLS blocks and routing any leftover rows/columns to
// the scalar fallback.
static void MultiplyRowBlock(const double* restrict A, size_t inner_dim,
                              const double* restrict B, size_t num_cols,
                              double* restrict C, size_t row_start,
                              size_t row_end, size_t k_start, size_t k_end) {
  const size_t tiled_row_end =
      row_start + ((row_end - row_start) / TILE_ROWS) * TILE_ROWS;
  const size_t tiled_col_end = (num_cols / TILE_COLS) * TILE_COLS;

  for (size_t row = row_start; row < tiled_row_end; row += TILE_ROWS) {
    for (size_t col = 0; col < tiled_col_end; col += TILE_COLS) {
      ComputeTile(A, inner_dim, B, num_cols, C, row, k_start, k_end, col);
    }
    if (tiled_col_end < num_cols) {
      ComputeTileScalar(A, inner_dim, B, num_cols, C, row, row + TILE_ROWS,
                         k_start, k_end, tiled_col_end, num_cols);
    }
  }
  if (tiled_row_end < row_end) {
    ComputeTileScalar(A, inner_dim, B, num_cols, C, tiled_row_end, row_end,
                       k_start, k_end, 0, num_cols);
  }
}

double* MatMat(const double* restrict A, const size_t A_rows,
               const size_t A_cols, const double* restrict B,
               const size_t B_rows, const size_t B_cols) {
  (void)B_rows;
  const size_t num_rows = A_rows;
  const size_t inner_dim = A_cols;
  const size_t num_cols = B_cols;

  double* restrict C = calloc(num_rows * num_cols, sizeof(double));
  MultiplyRowBlock(A, inner_dim, B, num_cols, C, 0, num_rows, 0, inner_dim);

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
