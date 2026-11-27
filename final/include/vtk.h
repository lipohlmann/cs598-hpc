#pragma once

#include <mpi.h>

#include "mesh.h"
#include "operator.h"
#include "quad_set.h"
#include "vector.h"

/**
 * @file vtk.h
 * @brief Parallel VTK output (XML UnstructuredGrid, one piece per rank).
 *
 * Every rank writes `<prefix>_<rank>.vtu` with its own elements, and rank 0
 * writes `<prefix>.pvtu`, which lists all pieces. Open the .pvtu in ParaView
 * or VisIt. Each spectral element is split into (n-1)^3 linear hexahedra whose
 * corners are the GLL nodes, so the GLL points are the points of the grid.
 * Nodes shared between elements are written once per element.
 *
 * Point data: `u` (numerical solution), `u_exact`, `error` (u - u_exact).
 * Cell data: `rank` (owning MPI rank, i.e. the RCB partition) and
 * `element_id` (global spectral element ID).
 *
 * Data are written as ASCII, so files are large for big meshes (roughly 100
 * bytes per GLL node).
 */

/**
 * @brief Write the solution of this rank to VTK. Collective.
 *
 * @param prefix Output path without extension, e.g. "results/vtk/finest".
 * The directory must exist.
 * @param op Operator (local elements and n)
 * @param mesh Mesh
 * @param quad GLL quadrature set
 * @param u Numerical solution (local layout)
 * @param u_exact Exact solution (local layout)
 * @param comm Communicator
 * @return 0 on success on every rank, nonzero if any rank failed
 */
int write_vtu_parallel(const char* prefix, const Operator* op, const Mesh* mesh,
                       const QuadSet* quad, const Vector* u,
                       const Vector* u_exact, MPI_Comm comm);
