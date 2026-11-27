#include "vtk.h"

#include <stdio.h>
#include <string.h>

#include "kronecker.h"

// part of prefix after the last '/', so the .pvtu can name its pieces
// relative to its own directory
static const char* base_name(const char* path) {
  const char* slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

static int write_piece(const char* filename, const Operator* op,
                       const Mesh* mesh, const QuadSet* quad, const Vector* u,
                       const Vector* u_exact, int rank) {
  FILE* f = fopen(filename, "w");
  if (!f) return 1;

  const size_t n = op->n;
  const size_t n3 = n * n * n;
  const size_t m = n - 1;  // sub-cells per direction per element
  const size_t n_points = op->n_elements * n3;
  const size_t n_cells = op->n_elements * m * m * m;

  fprintf(f, "<?xml version=\"1.0\"?>\n");
  fprintf(f,
          "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" "
          "byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
  fprintf(f, "<UnstructuredGrid>\n");
  fprintf(f, "<Piece NumberOfPoints=\"%zu\" NumberOfCells=\"%zu\">\n",
          n_points, n_cells);

  // points: local GLL nodes in the same layout as the solution vectors
  fprintf(f, "<Points>\n");
  fprintf(f,
          "<DataArray type=\"Float64\" NumberOfComponents=\"3\" "
          "format=\"ascii\">\n");
  for (size_t le = 0; le < op->n_elements; le++)
    for (size_t k = 0; k < n; k++)
      for (size_t j = 0; j < n; j++)
        for (size_t i = 0; i < n; i++) {
          double xyz[3];
          gll_node_position(mesh, quad, op->element_ids[le], i, j, k, xyz);
          fprintf(f, "%.10e %.10e %.10e\n", xyz[0], xyz[1], xyz[2]);
        }
  fprintf(f, "</DataArray>\n</Points>\n");

  // cells: linear hexahedra between neighbouring GLL nodes, VTK ordering
  // (bottom face counter-clockwise, then top face)
  fprintf(f, "<Cells>\n");
  fprintf(f, "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n");
  for (size_t le = 0; le < op->n_elements; le++) {
    const size_t base = le * n3;
    for (size_t k = 0; k < m; k++)
      for (size_t j = 0; j < m; j++)
        for (size_t i = 0; i < m; i++) {
          fprintf(f, "%zu %zu %zu %zu %zu %zu %zu %zu\n",
                  base + FlatIndex(i, j, k, n),
                  base + FlatIndex(i + 1, j, k, n),
                  base + FlatIndex(i + 1, j + 1, k, n),
                  base + FlatIndex(i, j + 1, k, n),
                  base + FlatIndex(i, j, k + 1, n),
                  base + FlatIndex(i + 1, j, k + 1, n),
                  base + FlatIndex(i + 1, j + 1, k + 1, n),
                  base + FlatIndex(i, j + 1, k + 1, n));
        }
  }
  fprintf(f, "</DataArray>\n");
  fprintf(f, "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n");
  for (size_t c = 1; c <= n_cells; c++) fprintf(f, "%zu\n", 8 * c);
  fprintf(f, "</DataArray>\n");
  fprintf(f, "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n");
  for (size_t c = 0; c < n_cells; c++) fprintf(f, "12\n");  // VTK_HEXAHEDRON
  fprintf(f, "</DataArray>\n</Cells>\n");

  // point data
  fprintf(f, "<PointData Scalars=\"u\">\n");
  const char* names[3] = {"u", "u_exact", "error"};
  for (int field = 0; field < 3; field++) {
    fprintf(f, "<DataArray type=\"Float64\" Name=\"%s\" format=\"ascii\">\n",
            names[field]);
    for (size_t p = 0; p < n_points; p++) {
      const double value = field == 0   ? u->data[p]
                           : field == 1 ? u_exact->data[p]
                                        : u->data[p] - u_exact->data[p];
      fprintf(f, "%.10e\n", value);
    }
    fprintf(f, "</DataArray>\n");
  }
  fprintf(f, "</PointData>\n");

  // cell data
  fprintf(f, "<CellData Scalars=\"rank\">\n");
  fprintf(f, "<DataArray type=\"Int32\" Name=\"rank\" format=\"ascii\">\n");
  for (size_t c = 0; c < n_cells; c++) fprintf(f, "%d\n", rank);
  fprintf(f, "</DataArray>\n");
  fprintf(f, "<DataArray type=\"Int64\" Name=\"element_id\" format=\"ascii\">\n");
  for (size_t le = 0; le < op->n_elements; le++)
    for (size_t c = 0; c < m * m * m; c++)
      fprintf(f, "%zu\n", op->element_ids[le]);
  fprintf(f, "</DataArray>\n</CellData>\n");

  fprintf(f, "</Piece>\n</UnstructuredGrid>\n</VTKFile>\n");
  return fclose(f) != 0;
}

static int write_index(const char* filename, const char* piece_base,
                       int n_ranks) {
  FILE* f = fopen(filename, "w");
  if (!f) return 1;
  fprintf(f, "<?xml version=\"1.0\"?>\n");
  fprintf(f,
          "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\" "
          "byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
  fprintf(f, "<PUnstructuredGrid GhostLevel=\"0\">\n");
  fprintf(f, "<PPoints>\n<PDataArray type=\"Float64\" NumberOfComponents=\"3\"/>\n</PPoints>\n");
  fprintf(f, "<PPointData Scalars=\"u\">\n");
  fprintf(f, "<PDataArray type=\"Float64\" Name=\"u\"/>\n");
  fprintf(f, "<PDataArray type=\"Float64\" Name=\"u_exact\"/>\n");
  fprintf(f, "<PDataArray type=\"Float64\" Name=\"error\"/>\n");
  fprintf(f, "</PPointData>\n");
  fprintf(f, "<PCellData Scalars=\"rank\">\n");
  fprintf(f, "<PDataArray type=\"Int32\" Name=\"rank\"/>\n");
  fprintf(f, "<PDataArray type=\"Int64\" Name=\"element_id\"/>\n");
  fprintf(f, "</PCellData>\n");
  for (int r = 0; r < n_ranks; r++)
    fprintf(f, "<Piece Source=\"%s_%d.vtu\"/>\n", piece_base, r);
  fprintf(f, "</PUnstructuredGrid>\n</VTKFile>\n");
  return fclose(f) != 0;
}

int write_vtu_parallel(const char* prefix, const Operator* op, const Mesh* mesh,
                       const QuadSet* quad, const Vector* u,
                       const Vector* u_exact, MPI_Comm comm) {
  int rank, size;
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  char filename[4096];
  int failed = 0;

  snprintf(filename, sizeof(filename), "%s_%d.vtu", prefix, rank);
  failed |= write_piece(filename, op, mesh, quad, u, u_exact, rank);

  if (rank == 0) {
    snprintf(filename, sizeof(filename), "%s.pvtu", prefix);
    failed |= write_index(filename, base_name(prefix), size);
  }

  int any_failed = 0;
  MPI_Allreduce(&failed, &any_failed, 1, MPI_INT, MPI_LOR, comm);
  return any_failed;
}
