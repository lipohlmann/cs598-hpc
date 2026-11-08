#include "mesh.h"

void create_mesh(Mesh* mesh, const double Lx, const double Ly, const double Lz,
                 const size_t nx, const size_t ny, const size_t nz) {
  mesh->nx = nx + 1;
  mesh->ny = ny + 1;
  mesh->nz = nz + 1;

  mesh->dx = Lx / (double)nx;
  mesh->dy = Ly / (double)ny;
  mesh->dz = Lz / (double)nz;

  mesh->xs = create_equispaced_values(mesh->nx, mesh->dx);
  mesh->ys = create_equispaced_values(mesh->ny, mesh->dy);
  mesh->zs = create_equispaced_values(mesh->nz, mesh->dz);

  mesh->node_ids = create_node_ids(mesh->nx, mesh->ny, mesh->nz);
}

void free_mesh(Mesh* mesh) {
  free(mesh->xs);
  mesh->xs = NULL;

  free(mesh->ys);
  mesh->ys = NULL;

  free(mesh->zs);
  mesh->zs = NULL;

  free(mesh->node_ids);
  mesh->node_ids = NULL;
}

double* create_equispaced_values(const size_t n_points, const double delta) {
  double* array = calloc(n_points, sizeof(double));
  for (size_t i = 0; i < n_points; i++) {
    array[i] = (double)i * delta;
  }
  return array;
}

size_t* create_node_ids(const size_t nx, const size_t ny, const size_t nz) {
  size_t* node_ids = calloc(nx * ny * nz, sizeof(size_t));
  for (size_t i = 0; i < nx * ny * nz; i++) node_ids[i] = i;
  return node_ids;
}

size_t flat_node_id(const size_t i, const size_t j, const size_t k,
                    const size_t nx, const size_t ny) {
  return k * nx * ny + j * nx + i;
}

size_t flat_node_id_from_triple(const Triple* triple, const size_t nx,
                                const size_t ny) {
  return flat_node_id(triple->i, triple->j, triple->k, nx, ny);
}