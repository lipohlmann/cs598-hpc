#include "operator.h"

#include "cg.h"
#include "kronecker.h"

void apply_A(Vector* y, const Vector* x, void* ctx) {
  const Operator* op = ctx;
  const size_t n3 = op->n * op->n * op->n;

  // 1. local operator, one element at a time
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

  // 2. sum contributions at shared nodes
  direct_stiffness_sum(y, op->gsh);

  // 3. zero the Dirichlet rows
  for (size_t i = 0; i < y->size; i++) y->data[i] *= op->mask.data[i];
}

double global_dot(const Vector* a, const Vector* b, void* ctx) {
  const Operator* op = ctx;

  double local_sum = 0.0;
  for (size_t i = 0; i < a->size; i++)
    local_sum += a->data[i] * b->data[i] * op->inv_mult.data[i];

  double global_sum = 0.0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, op->mpi_comm);
  return global_sum;
}