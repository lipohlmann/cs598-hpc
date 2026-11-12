#include "gather_scatter.h"

#include "kronecker.h"
#include "timers.h"

struct gs_data* create_gather_scatter(const Mesh* mesh, const size_t n,
                                      const size_t* element_ids,
                                      const size_t n_local_elements,
                                      const struct comm* comm) {
  const size_t n_local = n_local_elements * n * n * n;

  slong* ids = create_gll_global_ids(mesh, n, element_ids, n_local_elements);
  struct gs_data* gsh = gs_setup(ids, (uint)n_local, comm, 0, gs_auto, 0);
  free(ids);
  return gsh;
}

void direct_stiffness_sum(Vector* u, struct gs_data* gsh) {
  timer_start(TIMER_GATHER_SCATTER);
  gs(u->data, gs_double, gs_add, 0, gsh, NULL);
  timer_stop(TIMER_GATHER_SCATTER);
}

Vector create_inverse_multiplicity(const size_t size, struct gs_data* gsh) {
  Vector inv_mult = create_vector(size);
  for (size_t i = 0; i < size; i++) inv_mult.data[i] = 1.0;
  direct_stiffness_sum(&inv_mult, gsh);
  for (size_t i = 0; i < size; i++) inv_mult.data[i] = 1.0 / inv_mult.data[i];
  return inv_mult;
}

slong* create_gll_global_ids(const Mesh* mesh, const size_t n,
                             const size_t* element_ids,
                             const size_t n_local_elements) {
  const size_t ex = mesh->nx - 1, ey = mesh->ny - 1;
  const size_t N = n - 1;                         // polynomial order
  const size_t Gx = ex * N + 1, Gy = ey * N + 1;  // global GLL points in x, y
  const size_t n3 = n * n * n;

  slong* ids = malloc((n_local_elements * n3 + 1) * sizeof(*ids));
  for (size_t le = 0; le < n_local_elements; le++) {
    const size_t e = element_ids[le];
    const size_t ie = e % ex, je = (e / ex) % ey, ke = e / (ex * ey);
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++) {
          const size_t gi = ie * N + i, gj = je * N + j, gk = ke * N + k;
          ids[le * n3 + FlatIndex(i, j, k, n)] =
              1 + (slong)(gi + gj * Gx + gk * Gx * Gy);
        }
  }
  return ids;
}

Vector create_dirichlet_mask(const Mesh* mesh, const size_t n,
                             const size_t* element_ids,
                             const size_t n_local_elements) {
  const size_t ex = mesh->nx - 1, ey = mesh->ny - 1, ez = mesh->nz - 1;
  const size_t N = n - 1;
  const size_t Gx = ex * N + 1, Gy = ey * N + 1, Gz = ez * N + 1;
  const size_t n3 = n * n * n;

  Vector mask = create_vector(n_local_elements * n3);
  for (size_t le = 0; le < n_local_elements; le++) {
    const size_t e = element_ids[le];
    const size_t ie = e % ex, je = (e / ex) % ey, ke = e / (ex * ey);
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++) {
          const size_t gi = ie * N + i, gj = je * N + j, gk = ke * N + k;
          const int on_boundary = gi == 0 || gi == Gx - 1 || gj == 0 ||
                                  gj == Gy - 1 || gk == 0 || gk == Gz - 1;
          mask.data[le * n3 + FlatIndex(i, j, k, n)] = on_boundary ? 0.0 : 1.0;
        }
  }
  return mask;
}
