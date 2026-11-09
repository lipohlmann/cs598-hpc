#include "tridiag.h"

#include <mpi.h>

// First row and size of rank id's interior block. The N - (P - 1) interior
// rows are split as evenly as possible, and block id is followed by
// separator id (except for the last block).
static void Partition(int id, int P, int N, int* first, int* m) {
  int interior = N - (P - 1);
  int base = interior / P, extra = interior % P;
  *m = base + (id < extra ? 1 : 0);
  // the blocks before mine, plus the separator after each of them
  *first = id * base + (id < extra ? id : extra) + id;
}

void DomainDecompositionRows(int id, int P, int N, int* first, int* count) {
  int m;
  Partition(id, P, N, first, &m);
  *count = m + (id < P - 1 ? 1 : 0);
}

// Plain serial Thomas algorithm, used for the separator system. b and c have
// n - 1 entries.
static void SerialThomas(int n, const double* b, const double* a,
                         const double* c, const double* q, double* x) {
  double* d = new double[n];

  // factor and forward substitution together, using x for y
  d[0] = a[0];
  x[0] = q[0];
  for (auto i = 1; i < n; i++) {
    double l = b[i - 1] / d[i - 1];
    d[i] = a[i] - l * c[i - 1];
    x[i] = q[i] - l * x[i - 1];
  }

  // backward substitution
  x[n - 1] /= d[n - 1];
  for (auto i = n - 2; i >= 0; i--) x[i] = (x[i] - c[i] * x[i + 1]) / d[i];

  delete[] d;
}

void DomainDecompositionParallel(int my_id, int P, int N, double* b, double* a,
                                 double* c, double* x, double* q) {
  int first, m;
  double *d, *g, *vL, *vR;
  double S[6], *S_all, xsL, xsR;

  // find *my* rows: interior block first..last, then my right separator at
  // last + 1. Rank 0 has no left separator, rank P-1 no right one
  Partition(my_id, P, N, &first, &m);
  int last = first + m - 1;
  bool has_left = my_id > 0;
  bool has_right = my_id < P - 1;

  d = new double[m];
  g = new double[m];
  vL = new double[m];
  vR = new double[m];

  // Writing my block as A_p, its interior unknowns satisfy
  //   A_p x_p = q_p - bL e_1 xsL - cR e_m xsR,
  // so x_p = g - vL xsL - vR xsR with g = A_p^{-1} q_p, vL = A_p^{-1} bL e_1
  // and vR = A_p^{-1} cR e_m. Get all three from one factorization of A_p.
  double bL = has_left ? b[first - 1] : 0;  // my first row -> left separator
  double cR = has_right ? c[last] : 0;      // my last row -> right separator

  // factor A_p = LU and forward-substitute all three right-hand sides
  d[0] = a[first];
  g[0] = q[first];
  vL[0] = bL;
  vR[0] = 0;
  for (auto i = 1; i < m; i++) {
    double l = b[first + i - 1] / d[i - 1];
    d[i] = a[first + i] - l * c[first + i - 1];
    g[i] = q[first + i] - l * g[i - 1];
    vL[i] = -l * vL[i - 1];
    vR[i] = 0;
  }
  vR[m - 1] = cR;  // e_m is zero above row m, so forward leaves it unchanged

  // backward substitution for all three
  g[m - 1] /= d[m - 1];
  vL[m - 1] /= d[m - 1];
  vR[m - 1] /= d[m - 1];
  for (auto i = m - 2; i >= 0; i--) {
    g[i] = (g[i] - c[first + i] * g[i + 1]) / d[i];
    vL[i] = (vL[i] - c[first + i] * vL[i + 1]) / d[i];
    vR[i] = (vR[i] - c[first + i] * vR[i + 1]) / d[i];
  }

  // My contribution to the Schur complement, a 2x2 block on (left separator,
  // right separator) plus two rhs entries: {S11, S12, S21, S22, f1, f2}.
  // Substituting x_p into the separator equations gives the terms below; I
  // also own the right separator's own diagonal and rhs.
  for (auto i = 0; i < 6; i++) S[i] = 0;
  if (has_left) {
    double cl = c[first - 1];  // left separator row -> my first row
    S[0] = -cl * vL[0];
    S[1] = -cl * vR[0];
    S[4] = -cl * g[0];
  }
  if (has_right) {
    double br = b[last];  // right separator row -> my last row
    S[2] = -br * vL[m - 1];
    S[3] = a[last + 1] - br * vR[m - 1];
    S[5] = q[last + 1] - br * g[m - 1];
  }

  // everyone gets every block, then solves the (P-1) x (P-1) separator
  // system redundantly, which is cheaper than solving it once and sending
  // the answers back out
  S_all = new double[6 * P];
  MPI_Allgather(S, 6, MPI_DOUBLE, S_all, 6, MPI_DOUBLE, MPI_COMM_WORLD);

  xsL = 0;
  xsR = 0;
  int ns = P - 1;
  if (ns > 0) {
    double* sb = new double[ns];
    double* sa = new double[ns];
    double* sc = new double[ns];
    double* sq = new double[ns];
    double* xs = new double[ns];

    // separator j sits between blocks j and j+1, which both contribute
    for (auto j = 0; j < ns; j++) {
      sa[j] = S_all[6 * j + 3] + S_all[6 * (j + 1) + 0];
      sq[j] = S_all[6 * j + 5] + S_all[6 * (j + 1) + 4];
    }
    // block j+1 couples separators j and j+1
    for (auto j = 0; j < ns - 1; j++) {
      sc[j] = S_all[6 * (j + 1) + 1];
      sb[j] = S_all[6 * (j + 1) + 2];
    }
    SerialThomas(ns, sb, sa, sc, sq, xs);

    if (has_left) xsL = xs[my_id - 1];
    if (has_right) xsR = xs[my_id];

    delete[] sb;
    delete[] sa;
    delete[] sc;
    delete[] sq;
    delete[] xs;
  }

  // back substitution for my block, fully parallel now
  for (auto i = 0; i < m; i++) x[first + i] = g[i] - vL[i] * xsL - vR[i] * xsR;
  if (has_right) x[last + 1] = xsR;

  // cleanup
  delete[] d;
  delete[] g;
  delete[] vL;
  delete[] vR;
  delete[] S_all;
  return;
}
