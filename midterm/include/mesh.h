#pragma once

#include <stdlib.h>

#include "element.h"
#include "quad_set.h"
#include "triple.h"

typedef struct {
  size_t nx;  // number of points in the x-direction
  size_t ny;  // number of points in the y-direction
  size_t nz;  // number of points in the z-direction

  double dx;  // x grid spacing
  double dy;  // y grid spacing
  double dz;  // z grid spacing

  double* xs;  // array of node x-values
  double* ys;  // array of node y-values
  double* zs;  // array of node z-values

  size_t* node_ids;  // array of flat node IDs
                     // (to be indexed with flat_node_id())

  Element* elements;  // array of elements in mesh
} Mesh;

/**
 * @brief Create a mesh object. Note that the mesh always has its lower left
 * corner on the origin (0,0,0).
 *
 * @param Lx Length in x
 * @param Ly Length in y
 * @param Lz Length in z
 * @param nx Number of ELEMENTS in x
 * @param ny Number of ELEMENTS in y
 * @param nz Number of ELEMENTS in z
 * @return Mesh
 */
void create_mesh(Mesh* mesh, const double Lx, const double Ly, const double Lz,
                 const size_t nx, const size_t ny, const size_t nz);

/**
 * @brief Free the memory of all pointers in a mesh
 *
 * @param mesh
 */
void free_mesh(Mesh* mesh);

/**
 * @brief Create an array equispaced values
 *
 * @param n_points Total number of points
 * @param delta Spacing between points
 * @return double*
 */
double* create_equispaced_values(const size_t n_points, const double delta);

/**
 * @brief Create an array of flat node IDs, which is just a tensor product of
 * the number of values in each direction in the mesh
 *
 * @param nx Number of points in x
 * @param ny Number of points in x
 * @param nz Number of points in x
 * @return size_t*
 */
size_t* create_node_ids(const size_t nx, const size_t ny, const size_t nz);

/**
 * @brief Computes the equivalent flat node ID given an i,j,k triple of values
 *
 * @param i x index
 * @param j y index
 * @param k z index
 * @param nx Number of points in x
 * @param ny Number of points in z
 * @return size_t
 */
size_t flat_node_id(const size_t i, const size_t j, const size_t k,
                    const size_t nx, const size_t ny);

/**
 * @brief Does the same as flat_node_id(), except it takes a Triple data
 * structure instead of individual indices
 *
 * @param triple Triple of indices
 * @param nx Number of points in x
 * @param ny Number of points in z
 * @return size_t
 */
size_t flat_node_id_from_triple(const Triple* triple, const size_t nx,
                                const size_t ny);

/**
 * @brief Set the elements in the mesh based on the canonical form:
 *
 *    8 --------------- 7
 *   /|                /|
 *  / |               / |
 * 5 --------------- 6  |     +ζ
 * |  |              |  |      |  _+η
 * |  4 -------------|- 3      | /
 * | /               | /       |/___ +ξ
 * |/                |/
 * 1 --------------- 2

 *
 * @param mesh
 */
Element* create_elements(const size_t nx, const size_t ny, const size_t nz);

/**
 * @brief Get the equivalent triple of indices given a flat index
 *
 * @param node_id
 * @param mesh
 * @return Triple
 */
Triple flat_node_id_to_triple(const size_t node_id, const Mesh* mesh);

/**
 * @brief Physical position of a GLL node. The reference coordinate
 * \f$\xi\in[-1,1]\f$ of GLL point i is mapped to the element as
 * \f$x = x_e + \tfrac{1}{2}(\xi_i+1)\,dx\f$ (same for y and z).
 *
 * @param mesh Mesh
 * @param quad GLL quadrature set (reference node positions)
 * @param element_id Global element ID
 * @param i GLL index in x
 * @param j GLL index in y
 * @param k GLL index in z
 * @param xyz Output position
 */
void gll_node_position(const Mesh* mesh, const QuadSet* quad,
                       const size_t element_id, const size_t i, const size_t j,
                       const size_t k, double xyz[3]);
