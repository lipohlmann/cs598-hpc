#include "operator.h"

#include "cg.h"
#include "kronecker.h"
#include "rcb.h"
#include "timers.h"

void apply_A(Vector* y, const Vector* x, void* ctx) {
  const Operator* op = ctx;
  const size_t n3 = op->n * op->n * op->n;
  timer_start(TIMER_MATVEC);

  // 1. local operator, one element at a time
  timer_start(TIMER_KRONECKER);
  for (size_t e = 0; e < op->n_elements; e++) {
    // non-owning view of this element's slice of x
    const Vector x_e = {.size = n3, .data = x->data + e * n3};
    double* y_e = y->data + e * n3;

    Vector kx = SerialTripleKroneckerProduct(&x_e, &op->Mz, &op->My, &op->Kx);
    Vector ky = SerialTripleKroneckerProduct(&x_e, &op->Mz, &op->Ky, &op->Mx);
    Vector kz = SerialTripleKroneckerProduct(&x_e, &op->Kz, &op->My, &op->Mx);
    Vector m = SerialTripleKroneckerProduct(&x_e, &op->Mz, &op->My, &op->Mx);

    for (size_t i = 0; i < n3; i++)
      y_e[i] = kx.data[i] + ky.data[i] + kz.data[i] + m.data[i];

    free_vector(&kx);
    free_vector(&ky);
    free_vector(&kz);
    free_vector(&m);
  }
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

void setup_operator_local(Operator* op, const Mesh* mesh, const int* owner,
                          MPI_Comm comm) {
  int rank;
  MPI_Comm_rank(comm, &rank);

  const size_t total = (mesh->nx - 1) * (mesh->ny - 1) * (mesh->nz - 1);
  op->element_ids = rcb_local_elements(owner, total, rank, &op->n_elements);

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
