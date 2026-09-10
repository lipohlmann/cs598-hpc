#include "mat_mat_multiply.h"

#ifdef _OPENMP
#include <omp.h>
#endif

// Register-resident accumulator tile shape (rows x cols).
#define TILE_ROWS 6
#define TILE_COLS 8

// Depth of the K-panel of B reused by every thread before moving on.
#define K_PANEL_DEPTH 112

static inline size_t Min(size_t a, size_t b) { return a < b ? a : b; }

static inline size_t RoundUpToMultiple(size_t value, size_t multiple) {
  size_t remainder = value % multiple;
  return remainder == 0 ? value : value + (multiple - remainder);
}

static int GetNumThreads(void) {
#ifdef _OPENMP
  return omp_get_max_threads();
#else
  return 1;
#endif
}

// One contiguous row block per thread, rounded up to a multiple of
// TILE_ROWS so the fast path in MultiplyRowBlock covers as much as
// possible without falling back to ComputeTileScalar.
static size_t RowsPerThread(size_t num_rows) {
  size_t threads = (size_t)GetNumThreads();
  return RoundUpToMultiple((num_rows + threads - 1) / threads, TILE_ROWS);
}

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
  const size_t row_block = RowsPerThread(num_rows);

  // The K panel is the outer, sequential loop: every thread reuses the
  // same panel of B while sweeping its row block across all of C, instead
  // of re-reading B once per row block.
  for (size_t k_start = 0; k_start < inner_dim; k_start += K_PANEL_DEPTH) {
    const size_t k_end = Min(k_start + K_PANEL_DEPTH, inner_dim);

#pragma omp parallel for schedule(static)
    for (size_t row_start = 0; row_start < num_rows; row_start += row_block) {
      const size_t row_end = Min(row_start + row_block, num_rows);
      MultiplyRowBlock(A, inner_dim, B, num_cols, C, row_start, row_end,
                        k_start, k_end);
    }
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
