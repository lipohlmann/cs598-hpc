#pragma once

#include <mpi.h>
#include <stdio.h>

#include "gslib.h"
#include "mesh.h"
#include "vector.h"

/**
 * @brief Split the mesh elements into contiguous blocks of the flat element
 * ID, one block per rank. The first (total % size) ranks get one extra.
 */
void partition_elements(const Mesh* mesh, const int rank, const int size,
                        size_t* first_element, size_t* n_local_elements);

/**
 * @brief Global GLL node IDs (1-based, for gslib) for the elements
 * [first_element, first_element + n_local_elements). Caller frees.
 */
slong* create_gll_global_ids(const Mesh* mesh, const size_t n,
                             const size_t first_element,
                             const size_t n_local_elements);

/**
 * @brief Build the gslib handle for this rank's elements. Collective.
 */
struct gs_data* create_gather_scatter(const Mesh* mesh, const size_t n,
                                      const size_t first_element,
                                      const size_t n_local_elements,
                                      const struct comm* comm);

/**
 * @brief Dirichlet mask for this rank's elements: 0 on the domain boundary
 * (all six faces), 1 elsewhere.
 */
Vector create_dirichlet_mask(const Mesh* mesh, const size_t n,
                             const size_t first_element,
                             const size_t n_local_elements);

void direct_stiffness_sum(Vector* u, struct gs_data* gsh);
Vector create_inverse_multiplicity(const size_t size, struct gs_data* gsh);
