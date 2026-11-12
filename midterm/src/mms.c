#include "mms.h"

#include <math.h>

#include "gather_scatter.h"
#include "kronecker.h"
#include "matrix.h"

double mms_u_exact(double x, double y, double z, void* params) {
  const MmsParams* p = params;
  return sin(4.0 * M_PI * x / p->Lx) * sin(2.0 * M_PI * y / p->Ly) *
         sin(3.0 * M_PI * z / p->Lz);
}

double mms_forcing(double x, double y, double z, void* params) {
  const MmsParams* p = params;
  const double k2 =
      M_PI * M_PI *
      (16.0 / (p->Lx * p->Lx) + 4.0 / (p->Ly * p->Ly) + 9.0 / (p->Lz * p->Lz));
  return (k2 + 1.0) * mms_u_exact(x, y, z, params);
}

Vector evaluate_field(const Operator* op, const Mesh* mesh, const QuadSet* quad,
                      field_fn f, void* params) {
  const size_t n = op->n;
  const size_t n3 = n * n * n;

  Vector values = create_vector(op->n_elements * n3);
  for (size_t le = 0; le < op->n_elements; le++)
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++) {
          double xyz[3];
          gll_node_position(mesh, quad, op->element_ids[le], i, j, k, xyz);
          values.data[le * n3 + FlatIndex(i, j, k, n)] =
              f(xyz[0], xyz[1], xyz[2], params);
        }
  return values;
}

// lumped mass weight of local node (i,j,k): Mx[i] * My[j] * Mz[k]
static double mass_weight(const Operator* op, size_t i, size_t j, size_t k) {
  return matrix_value_at(i, i, &op->Mx) * matrix_value_at(j, j, &op->My) *
         matrix_value_at(k, k, &op->Mz);
}

Vector create_rhs(const Operator* op, const Mesh* mesh, const QuadSet* quad,
                  field_fn f, void* params) {
  const size_t n = op->n;
  const size_t n3 = n * n * n;

  Vector b = evaluate_field(op, mesh, quad, f, params);
  for (size_t le = 0; le < op->n_elements; le++)
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++)
          b.data[le * n3 + FlatIndex(i, j, k, n)] *= mass_weight(op, i, j, k);

  direct_stiffness_sum(&b, op->gsh);
  for (size_t i = 0; i < b.size; i++) b.data[i] *= op->mask.data[i];
  return b;
}

void mms_error_norms(const Operator* op, const Vector* u, const Vector* u_ex,
                     double* l2, double* linf) {
  const size_t n = op->n;
  const size_t n3 = n * n * n;

  double sum = 0.0, max = 0.0;
  for (size_t le = 0; le < op->n_elements; le++)
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++) {
          const size_t idx = le * n3 + FlatIndex(i, j, k, n);
          const double diff = u->data[idx] - u_ex->data[idx];
          // inv_mult makes each shared node count once
          sum +=
              mass_weight(op, i, j, k) * op->inv_mult.data[idx] * diff * diff;
          if (fabs(diff) > max) max = fabs(diff);
        }

  double global_sum = 0.0, global_max = 0.0;
  MPI_Allreduce(&sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, op->mpi_comm);
  MPI_Allreduce(&max, &global_max, 1, MPI_DOUBLE, MPI_MAX, op->mpi_comm);
  *l2 = sqrt(global_sum);
  *linf = global_max;
}
