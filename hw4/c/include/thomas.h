#pragma once

/**
 * @brief A paralel Thomas algorithm function for tridiagonal,
 * SPD/diagonally-dominant matrices. Function assumes MPI initialization has
 * already been done, and it requires the number of processes used is a power of
 * two.
 *
 * Algorithm taken from
 * https://yangpl.wordpress.com/wp-content/uploads/2017/12/parallel-scientific-computing-in-c-and-mpi-2003.pdf,
 * pages 371-374.
 *
 * @param id Process ID number
 * @param P Total number of processors being used
 * @param N Size of the matrix
 * @param b Subdiagonal array of matrix values
 * @param a Diagonal value of matrix values
 * @param c Superdiagonal array of matrix values
 * @param x Solution vector
 * @param q RHS vector
 */
void ThomasAlgorithmParallel(int id, int P, int N, double* b, double* a,
                             double* c, double* x, double* q);