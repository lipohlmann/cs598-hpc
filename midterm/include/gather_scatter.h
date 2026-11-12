#pragma once

#include <mpi.h>
#include <stdio.h>

#include "gslib.h"
#include "mesh.h"
#include "vector.h"

slong* create_gll_global_ids(const Mesh* mesh, const size_t n);
struct gs_data* create_gather_scatter(const Mesh* mesh, const size_t n,
                                      struct comm* comm);
void direct_stiffness_sum(Vector* u, struct gs_data* gsh);
Vector create_inverse_multiplicity(const size_t size, struct gs_data* gsh);