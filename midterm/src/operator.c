#include "operator.h"

#include "cg.h"
#include "kronecker.h"
#include "timers.h"

void local_operator_apply(const Operator* op, const double* restrict u,
                          double* restrict y) {
  const size_t n = op->n;
  const size_t n2 = n * n;
  const double* restrict kx = op->Kx.data;  // dense, row-major n x n
  const double* restrict ky = op->Ky.data;
  const double* restrict kz = op->Kz.data;
  const double* restrict mx = op->Mx.data;  // diagonals, length n
  const double* restrict my = op->My.data;
  const double* restrict mz = op->Mz.data;

  // x-direction stiffness and the mass term, one x-line (fixed j, k) at a time:
  // y[i,j,k] = my[j] mz[k] (sum_p Kx[i,p] u[p,j,k] + mx[i] u[i,j,k])
  for (size_t k = 0; k < n; k++)
    for (size_t j = 0; j < n; j++) {
      const double* restrict u_line = u + j * n + k * n2;
      double* restrict y_line = y + j * n + k * n2;
      const double myz = my[j] * mz[k];
      for (size_t i = 0; i < n; i++) {
        double sum = mx[i] * u_line[i];
        for (size_t p = 0; p < n; p++) sum += kx[i * n + p] * u_line[p];
        y_line[i] = myz * sum;
      }
    }

  // y-direction stiffness: y[i,j,k] += mx[i] mz[k] sum_p Ky[j,p] u[i,p,k]
  // (innermost loop over i: unit stride)
  for (size_t k = 0; k < n; k++)
    for (size_t j = 0; j < n; j++) {
      double* restrict y_line = y + j * n + k * n2;
      for (size_t p = 0; p < n; p++) {
        const double a = mz[k] * ky[j * n + p];
        const double* restrict u_line = u + p * n + k * n2;
        for (size_t i = 0; i < n; i++) y_line[i] += a * mx[i] * u_line[i];
      }
    }

  // z-direction stiffness: y[i,j,k] += mx[i] my[j] sum_p Kz[k,p] u[i,j,p]
  for (size_t k = 0; k < n; k++)
    for (size_t p = 0; p < n; p++) {
      const double a = kz[k * n + p];
      for (size_t j = 0; j < n; j++) {
        const double b = a * my[j];
        const double* restrict u_line = u + j * n + p * n2;
        double* restrict y_line = y + j * n + k * n2;
        for (size_t i = 0; i < n; i++) y_line[i] += b * mx[i] * u_line[i];
      }
    }
}

void apply_A(Vector* y, const Vector* x, void* ctx) {
  const Operator* op = ctx;
  const size_t n3 = op->n * op->n * op->n;
  timer_start(TIMER_MATVEC);

  // 1. local operator, one element at a time
  timer_start(TIMER_KRONECKER);
  for (size_t e = 0; e < op->n_elements; e++)
    local_operator_apply(op, x->data + e * n3, y->data + e * n3);
  timer_stop(TIMER_KRONECKER);

  // 2. sum contributions at shared nodes
  direct_stiffness_sum(y, op->gsh);

  // 3. zero the Dirichlet rows
  for (size_t i = 0; i < y->size; i++) y->data[i] *= op->mask.data[i];
  timer_stop(TIMER_MATVEC);
}

double global_dot(const Vector* a, const Vector* b, void* ctx) {
  const Operator* op = ctx;

  double local_sum = 0.0;
  for (size_t i = 0; i < a->size; i++)
    local_sum += a->data[i] * b->data[i] * op->inv_mult.data[i];

  double global_sum = 0.0;
  timer_start(TIMER_ALLREDUCE);
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, op->mpi_comm);
  timer_stop(TIMER_ALLREDUCE);
  return global_sum;
}

void setup_operator_local(Operator* op, const Mesh* mesh, size_t* element_ids,
                          const size_t n_local, MPI_Comm comm) {
  op->element_ids = element_ids;
  op->n_elements = n_local;

  op->mpi_comm = comm;
  op->mask =
      create_dirichlet_mask(mesh, op->n, op->element_ids, op->n_elements);

  struct comm gs_comm;
  comm_init(&gs_comm, comm);
  op->gsh = create_gather_scatter(mesh, op->n, op->element_ids, op->n_elements,
                                  &gs_comm);
  comm_free(&gs_comm);

  op->inv_mult = create_inverse_multiplicity(
      op->n_elements * op->n * op->n * op->n, op->gsh);
}

void free_operator(Operator* op) {
  free_matrix(&op->Kx);
  free_matrix(&op->Ky);
  free_matrix(&op->Kz);
  free_matrix(&op->Mx);
  free_matrix(&op->My);
  free_matrix(&op->Mz);
  free(op->element_ids);
  op->element_ids = NULL;
  free_vector(&op->mask);
  free_vector(&op->inv_mult);
  if (op->gsh) gs_free(op->gsh);
  op->gsh = NULL;
}
