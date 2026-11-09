#pragma once

/**
 * @brief A parallel domain-decomposition solver for tridiagonal,
 * SPD/diagonally-dominant matrices. Function assumes MPI initialization has
 * already been done. Any number of processes works, as long as N >= 2P - 1.
 *
 * The rows are split into P interior blocks with one separator row between
 * neighbouring blocks. Each rank eliminates its own block, the P - 1 separator
 * values are found from a small tridiagonal Schur complement system that every
 * rank solves redundantly, and each rank then back-substitutes its own block.
 * The only communication is one MPI_Allgather of 6 doubles per rank.
 *
 * Every rank passes the full matrix and right-hand side, but only reads its own
 * rows and their neighbouring separators. On return x holds the solution on the
 * rows this rank owns (see DomainDecompositionRows); the rest is untouched.
 *
 * @param id Process ID number
 * @param P Total number of processors being used
 * @param N Size of the matrix
 * @param b Subdiagonal array of matrix values, b[i] = A[i+1][i]
 * @param a Diagonal value of matrix values
 * @param c Superdiagonal array of matrix values, c[i] = A[i][i+1]
 * @param x Solution vector
 * @param q RHS vector
 */
void DomainDecompositionParallel(int id, int P, int N, double* b, double* a,
                                 double* c, double* x, double* q);

/**
 * @brief The rows of x that DomainDecompositionParallel fills in on rank id:
 * its interior block followed by its right separator (the last rank has none).
 * The ranges of all ranks are contiguous and together cover 0..N-1.
 *
 * @param id Process ID number
 * @param P Total number of processors being used
 * @param N Size of the matrix
 * @param first First row owned by this rank
 * @param count Number of rows owned by this rank
 */
void DomainDecompositionRows(int id, int P, int N, int* first, int* count);
