#include "gather_scatter.h"

#include "kronecker.h"

struct gs_data* create_gather_scatter(const Mesh* mesh, const size_t n,
                                      struct comm* comm) {
  const size_t n_elements = (mesh->nx - 1) * (mesh->ny - 1) * (mesh->nz - 1);
  const size_t n_local = n_elements * n * n * n;

  slong* ids = create_gll_global_ids(mesh, n);
  struct gs_data* gsh = gs_setup(ids, (uint)n_local, comm, 0, gs_auto, 0);
  free(ids);
  return gsh;
}

void direct_stiffness_sum(Vector* u, struct gs_data* gsh) {
  gs(u->data, gs_double, gs_add, 0, gsh, NULL);
}

Vector create_inverse_multiplicity(const size_t size, struct gs_data* gsh) {
  Vector inv_mult = create_vector(size);
  for (size_t i = 0; i < size; i++) inv_mult.data[i] = 1.0;
  direct_stiffness_sum(&inv_mult, gsh);
  for (size_t i = 0; i < size; i++) inv_mult.data[i] = 1.0 / inv_mult.data[i];
  return inv_mult;
}

slong* create_gll_global_ids(const Mesh* mesh, const size_t n) {
  const size_t ex = mesh->nx - 1, ey = mesh->ny - 1, ez = mesh->nz - 1;
  const size_t N = n - 1;                          // polynomial order
  const size_t Gx = ex * N + 1, Gy = ey * N + 1;   // global GLL points in x, y
  const size_t n3 = n * n * n;

  slong* ids = malloc(ex * ey * ez * n3 * sizeof(*ids));
  for (size_t ke = 0; ke < ez; ke++)
    for (size_t je = 0; je < ey; je++)
      for (size_t ie = 0; ie < ex; ie++) {
        const size_t e = flat_node_id(ie, je, ke, ex, ey);
        for (size_t k = 0; k < n; k++)
          for (size_t j = 0; j < n; j++)
            for (size_t i = 0; i < n; i++) {
              const size_t gi = ie * N + i, gj = je * N + j, gk = ke * N + k;
              ids[e * n3 + FlatIndex(i, j, k, n)] =
                  1 + (slong)(gi + gj * Gx + gk * Gx * Gy);
            }
      }
  return ids;
}
