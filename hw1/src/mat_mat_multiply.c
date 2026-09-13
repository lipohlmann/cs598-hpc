#include "mat_mat_multiply.h"

// Register-resident accumulator tile shape (rows x cols), sized to the
// vector width -march=native actually enables so one row's accumulator
// fills whole vector registers instead of spilling or under-using them:
// AVX-512 packs 8 doubles/register, AVX2 packs 4. Matches the Makefile's
// -mprefer-vector-width choice for the same reason.
#if defined(__AVX512F__)
#define TILE_ROWS 8
#define TILE_COLS 8
#elif defined(__AVX2__) && defined(__FMA__)
#define TILE_ROWS 4
#define TILE_COLS 4
#else
#define TILE_ROWS 2
#define TILE_COLS 4
#endif

// Block sizes for the loop nest in MatMat. KC caps how many rows of B
// ([kc, kc+KC) x full width) are swept repeatedly while resident in
// cache/L3 -- in row-major storage that row-range of B is already one
// contiguous chunk of memory, so no packing copy is needed to get reuse
// out of it. MC caps how many rows of A (x KC cols) are pulled in per
// block so that block plus the KC-slab of B comfortably fit in cache
// alongside each other. Both are picked for a few-MB L2/L3 budget and are
// not re-derived from detected cache sizes, since -march=native isn't
// portable enough to assume a specific topology here.
#define KC 128
#define MC 96

static inline size_t Min(size_t a, size_t b) { return a < b ? a : b; }

// Accumulates the TILE_ROWS x TILE_COLS block of C at (row0, col0) over the
// reduction range [k0, k0+kdepth), in register-resident accumulators, so
// each element of A is read once per k (reused across TILE_COLS columns)
// and each element of B is read once per k (reused across TILE_ROWS rows).
// C is read once (existing partial sum from prior K-blocks) and written
// once per call, instead of every C entry re-walking A and B from scratch
// with no reuse. The inner loop is TILE_COLS wide to match one full vector
// register (see the TILE_COLS definition above); #pragma omp simd tells
// the vectorizer it may treat those TILE_COLS lanes as independent (each
// is its own reduction over k, so packing them into one vector op isn't a
// floating-point reordering of any single lane's sum).
static void ComputeTile(const double* restrict A, size_t A_cols,
                         const double* restrict B, size_t B_cols,
                         double* restrict C, size_t row0, size_t col0,
                         size_t k0, size_t kdepth) {
  const double* restrict a_row[TILE_ROWS];
  double* restrict c_row[TILE_ROWS];
  double acc[TILE_ROWS][TILE_COLS];
  for (size_t r = 0; r < TILE_ROWS; r++) {
    a_row[r] = A + (row0 + r) * A_cols + k0;
    c_row[r] = C + (row0 + r) * B_cols + col0;
    for (size_t c = 0; c < TILE_COLS; c++) acc[r][c] = c_row[r][c];
  }

  for (size_t k = 0; k < kdepth; k++) {
    const double* restrict b_row = B + (k0 + k) * B_cols + col0;
    for (size_t r = 0; r < TILE_ROWS; r++) {
      const double av = a_row[r][k];
#pragma omp simd
      for (size_t c = 0; c < TILE_COLS; c++) {
        acc[r][c] += av * b_row[c];
      }
    }
  }

  for (size_t r = 0; r < TILE_ROWS; r++)
    for (size_t c = 0; c < TILE_COLS; c++) c_row[r][c] = acc[r][c];
}

// Scalar fallback for row/column remainders that don't fill a full
// TILE_ROWS x TILE_COLS tile. Also accumulates into existing C values.
static void ComputeBlockScalar(const double* restrict A, size_t A_cols,
                                const double* restrict B, size_t B_cols,
                                double* restrict C, size_t row_start,
                                size_t row_end, size_t col_start,
                                size_t col_end, size_t k0, size_t kdepth) {
  for (size_t row = row_start; row < row_end; row++) {
    const double* restrict a_row = A + row * A_cols;
    double* restrict c_row = C + row * B_cols;
    for (size_t col = col_start; col < col_end; col++) {
      double sum = c_row[col];
      for (size_t k = 0; k < kdepth; k++) {
        sum += a_row[k0 + k] * B[(k0 + k) * B_cols + col];
      }
      c_row[col] = sum;
    }
  }
}

// Runs the tiled microkernel (plus row/column-remainder fallback) over
// rows [row_start, row_end) x all of B_cols, for the reduction range
// [k0, k0+kdepth).
static void MultiplyRowBlock(const double* restrict A, size_t A_cols,
                              const double* restrict B, size_t B_cols,
                              double* restrict C, size_t row_start,
                              size_t row_end, size_t k0, size_t kdepth) {
  const size_t row_tiled_end =
      row_start + ((row_end - row_start) / TILE_ROWS) * TILE_ROWS;
  const size_t col_tiled_end = (B_cols / TILE_COLS) * TILE_COLS;

  for (size_t row = row_start; row < row_tiled_end; row += TILE_ROWS) {
    for (size_t col = 0; col < col_tiled_end; col += TILE_COLS) {
      ComputeTile(A, A_cols, B, B_cols, C, row, col, k0, kdepth);
    }
    if (col_tiled_end < B_cols) {
      ComputeBlockScalar(A, A_cols, B, B_cols, C, row, row + TILE_ROWS,
                          col_tiled_end, B_cols, k0, kdepth);
    }
  }
  if (row_tiled_end < row_end) {
    ComputeBlockScalar(A, A_cols, B, B_cols, C, row_tiled_end, row_end, 0,
                        B_cols, k0, kdepth);
  }
}

double* MatMat(const double* restrict A, const size_t A_rows,
               const size_t A_cols, const double* restrict B,
               const size_t B_rows, const size_t B_cols) {
  (void)B_rows;
  double* restrict C = calloc(A_rows * B_cols, sizeof(double));

  // K is blocked on the outside so a KC-row slab of B -- already
  // contiguous in row-major storage -- is swept by every row-block of A
  // while it's still warm in cache, instead of the full height of B being
  // re-streamed from memory once per row-block. M is then blocked so each
  // A/C block worked on within a slab stays cache-resident too.
  for (size_t k0 = 0; k0 < A_cols; k0 += KC) {
    const size_t kdepth = Min(KC, A_cols - k0);
    for (size_t row_start = 0; row_start < A_rows; row_start += MC) {
      const size_t row_end = Min(row_start + MC, A_rows);
      MultiplyRowBlock(A, A_cols, B, B_cols, C, row_start, row_end, k0,
                        kdepth);
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
