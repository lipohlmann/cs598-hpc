#pragma once

#include <stdlib.h>

#include "mesh.h"

/**
 * @file rcb.h
 * @brief Recursive coordinate bisection (RCB) of the mesh elements across MPI
 * ranks.
 *
 * RCB is a geometric partitioner. It only looks at where the elements are (their
 * centroids), not at which elements touch, and it works like this:
 *
 *  1. Start with all elements and all P ranks.
 *  2. If P == 1, the whole set of elements belongs to that one rank. Done.
 *  3. Otherwise find the bounding box of the element centroids and pick its
 *     LONGEST axis (x, y or z). Cutting the longest side keeps subdomains
 *     compact (close to cubes) which minimizes the surface area, i.e. the
 *     number of nodes shared with other ranks that gather-scatter must
 *     exchange.
 *  4. Sort the elements along that axis (ties are broken by element ID so the
 *     result is deterministic) and cut the sorted list in two:
 *       P_left  = P / 2           (integer division)
 *       P_right = P - P_left
 *       the left part receives  count * P_left / P  elements (integer
 *       division), the right part receives the rest.
 *     Splitting the elements in proportion to the ranks they go to is what
 *     keeps the load balanced when P is not a power of two.
 *  5. Recurse on each part with its own share of the ranks. The left part
 *     gets ranks [first_rank, first_rank + P_left), the right part the rest.
 *
 * Example: 4x4x4 elements, P = 4. The first cut halves the longest axis (ties
 * go to x), giving two 2x4x4 halves with 2 ranks each. Each half is then
 * halved again along its longest remaining axis (y), giving four 2x2x4
 * columns of 16 elements.
 *
 * Cost is O(E log E log P) for E elements, because every one of the ceil(log2
 * P) levels sorts at most E items. If P > E some ranks receive no elements.
 *
 * Limitations: all elements have equal weight, and the cut quality is based on
 * geometry only (no graph/edge-cut optimization). The partition is static.
 */

/**
 * @brief Partition the elements of a mesh into n_parts parts with recursive
 * coordinate bisection (see the file description).
 *
 * @param mesh Mesh whose elements are partitioned
 * @param n_parts Number of parts, normally the number of MPI ranks (>=1)
 * @return int* Array of length (nx-1)(ny-1)(nz-1) giving the owning part of
 * each element, indexed by element ID. Release with free().
 */
int* rcb_partition(const Mesh* mesh, const int n_parts);

/**
 * @brief Group all element IDs by the part that owns them (counting sort).
 *
 * @param owner Result of rcb_partition()
 * @param n_elements Total number of elements in the mesh
 * @param n_parts Number of parts
 * @param counts Output array of n_parts entries: elements owned by each part
 * @return size_t* All element IDs, those of part 0 first, then part 1, ...;
 * ascending within a part. Release with free().
 */
size_t* rcb_group_by_part(const int* owner, const size_t n_elements,
                          const int n_parts, int* counts);
