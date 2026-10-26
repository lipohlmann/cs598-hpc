#pragma once

typedef struct Block {
  int rank, n_ranks;
  long g_first_node_index;
  int n_my_nodes;
  int has_left, has_right;

  double *a, *b, *c, *f;  // local copies of interior rows

  /* the "dragged along" vectors from the sweep (length m)     */
  double *uL; /* column of U for the left separator            */
  double *uR; /* column of U for the right separator           */
  double *lL; /* row of L^T for the left separator             */
  double *lR; /* row of L^T for the right separator            */

  /* my local 2x2 Schur block and rhs pieces                   */
  double S11, S12, S21, S22;
  double fS1, fS2;
} Block;

void serial_thomas(int n, const double *a, const double *b, const double *c,
                   const double *f, double *x);

/* ------------------------------------------------------------------ */
/* Stage 1a: decide who owns what                                      */
/* ------------------------------------------------------------------ */
/* Given n, P and rank p, compute: first, m, has_left, has_right.
 * Layout:   [ m_0 interior ][sep][ m_1 interior ][sep] ... [ m_{P-1} interior ]
 * Requirements: every global node owned exactly once, every m >= 1,
 * block sizes differ by at most 1.
 * HINT: first work out how many interior nodes there are in total
 *       (n minus the number of separators), then split them evenly.    */
void partition(long n, int P, int p, Block *blk);

/* ------------------------------------------------------------------ */
/* Stage 1b: the local sweep, notes p.13 "Main Loop"                    */
/* ------------------------------------------------------------------ */
/* Inputs : blk->a,b,c,f for my interior rows, plus the four separator
 *          couplings and the separator's own diagonal/rhs (see GUIDE s.3).
 * Outputs: blk->a,c,f overwritten by the factored/updated values,
 *          blk->uL,uR,lL,lR filled in, S11,S12,S21,S22,fS1,fS2 set.
 * Steps  :
 *   1. initialise uL,uR,lL,lR, S11,S22,fS1,fS2   (GUIDE s.3 table, s.5)
 *   2. for k = 0..m-2: the row operations from p.13
 *   3. finish with the last row (p.14), GUIDE s.5
 * Be careful with the order of the updates inside one iteration!       */
void local_sweep(Block *blk, double cL,
                 double bL_row,            /* couplings at the left sep   */
                 double bR, double cR_row, /* couplings at the right sep  */
                 double a_sepR, double f_sepR);

/* ------------------------------------------------------------------ */
/* Stage 1c: build and solve the separator system                      */
/* ------------------------------------------------------------------ */
/* Combine every rank's S11,S12,S21,S22,fS1,fS2 into the (P-1) x (P-1)
 * tridiagonal system, solve it with serial_thomas, return xs[0..P-2].
 * Stage 1 (no MPI): take arrays indexed by rank.
 * Stage 3 (MPI)  : replace the arrays by MPI_Allgather.                */
void solve_schur(int P,
                 /* ...whatever data you decide to pass... */ double *xs);

/* ------------------------------------------------------------------ */
/* Stage 1d: back-substitution for my interior (no communication)      */
/* ------------------------------------------------------------------ */
/* Given x of the left and right separators (ignore them if absent),
 * compute x for my m interior nodes, GUIDE s.7.                         */
void back_substitute(const Block *blk, double xsL, double xsR, double *xloc);

/* max_i | (A x - f)_i |   for the full system of size n (serial).      */
double residual_inf(int n, const double *a, const double *b, const double *c,
                    const double *f, const double *x);