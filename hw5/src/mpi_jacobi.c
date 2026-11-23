#include "mpi_jacobi.h"

#include <math.h>
#include <stdio.h>

#include "matrix.h"
#include "poisson.h"

void MPIJacobi(const size_t N, const double L, const int boundary_type,
               const double tol, const int max_iters) {
  // get number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get the rank of the process
  int this_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &this_rank);

  MPI_Status status;

  size_t n_local_rows = (int)floor((double)N / world_size);
  size_t local_offset = this_rank * n_local_rows;
  if (this_rank == world_size - 1)
    n_local_rows = N - n_local_rows * (world_size - 1);

  if (this_rank == 0) {
    // if rank 0, create Poisson matrix and broadcast to other ranks
    Matrix poisson_matrix = CreatePoissonMatrix(N, L);
    Vector boundary_vector = CreateBoundaryVector(N, L, boundary_type);
    for (int n = 1; n < world_size; n++) {
      for (size_t i = 0; i < N - 1; i++) {
        MPI_Send(&boundary_vector.data[i], 1, MPI_DOUBLE, n, i, MPI_COMM_WORLD);
        for (size_t j = 0; j < N - 1; j++) {
          size_t flat_index_ = flat_index(i, j, &poisson_matrix);
          MPI_Send(&poisson_matrix.data[flat_index_], 1, MPI_DOUBLE, n,
                   flat_index_, MPI_COMM_WORLD);
        }
      }
    }

  } else {
    // create empty matrix and vector of the correct size, receive data from
    // rank 0

    Matrix poisson_matrix = create_matrix(N - 1, N - 1, 0);
    Vector boundary_vector = create_vector(N - 1);

    for (size_t i = 0; i < N - 1; i++) {
      MPI_Recv(&boundary_vector.data[i], 1, MPI_DOUBLE, 0, i, MPI_COMM_WORLD,
               &status);
      for (size_t j = 0; j < N - 1; j++) {
        size_t flat_index_ = flat_index(i, j, &poisson_matrix);
        MPI_Recv(&poisson_matrix.data[flat_index_], 1, MPI_DOUBLE, 0,
                 flat_index_, MPI_COMM_WORLD, &status);
      }
    }

    Vector xk = create_vector(N - 1);
    Vector xkp1 = create_vector(N - 1);
    for (size_t i = 0; i < N - 1; i++) xk.data[i] = 1.0;  // initialize to 1's

    int k = 0;
    double residual_norm = 0.0;
    double local_residual_sqrd = 0.0;
    double global_residual = 0.0;
    while (k <= max_iters) {
      k++;
      // Jacobi method

      for (size_t i = local_offset; i < local_offset + n_local_rows; i++) {
        double diag = poisson_matrix.data[flat_index(i, i, &poisson_matrix)];
        double bi = boundary_vector.data[i];

        double sum = 0.0;
        for (size_t j = 0; j < i; j++)
          sum += poisson_matrix.data[flat_index(i, j, &poisson_matrix)] *
                 xk.data[j];
        for (size_t j = i + 1; j < N - 1; j++)
          sum += poisson_matrix.data[flat_index(i, j, &poisson_matrix)] *
                 xk.data[j];

        xkp1.data[i] = 1.0 / diag * (bi - sum);
        local_residual_sqrd += pow(bi - sum - diag * xk.data[i], 2.0);
      }
      // update global residual
      MPI_Allreduce(&local_residual_sqrd, &global_residual, 1, MPI_DOUBLE,
                    MPI_SUM, MPI_COMM_WORLD);
      residual_norm = sqrt(global_residual);
      printf("k = %d, residual_norm = %.4e\n", k, residual_norm);

      if (residual_norm < tol) {
        free_vector(&boundary_vector);
        free_matrix(&poisson_matrix);
        free_vector(&xk);
        free_vector(&xkp1);
        return;
      }
    }
  }
}