#pragma once

#include "mesh.h"
#include "operator.h"
#include "quad_set.h"
#include "vector.h"

/**
 * @file mms.h
 * @brief Method of manufactured solutions (MMS) for the operator
 * \f$-\nabla^2 u + u = f\f$ with homogeneous Dirichlet conditions.
 *
 * Pick an exact solution \f$u_{ex}\f$, derive the forcing analytically as
 * \f$f=-\nabla^2 u_{ex}+u_{ex}\f$, solve \f$A u = b\f$ with
 * \f$b = M f\f$ (M is the lumped GLL mass matrix), and compare \f$u\f$ with
 * \f$u_{ex}\f$. To use a different solution, edit mms_u_exact() and
 * mms_forcing() in src/mms.c. Because the operator enforces homogeneous
 * Dirichlet conditions, \f$u_{ex}\f$ must be 0 on all six faces of the domain.
 */

/**
 * @brief A scalar field evaluated at a point.
 *
 * @param x
 * @param y
 * @param z
 * @param params Opaque pointer to user data (an MmsParams for the functions
 * in this file)
 */
typedef double (*field_fn)(double x, double y, double z, void* params);

typedef struct {
  double Lx, Ly, Lz;  // domain lengths
} MmsParams;

/**
 * @brief Exact solution \f$\sin(4\pi x/L_x)\sin(2\pi y/L_y)\sin(3\pi z/L_z)\f$.
 */
double mms_u_exact(double x, double y, double z, void* params);

/**
 * @brief Forcing \f$f=-\nabla^2 u_{ex}+u_{ex}\f$ for mms_u_exact(), which is
 * \f$\left(\pi^2(16/L_x^2+4/L_y^2+9/L_z^2)+1\right)u_{ex}\f$.
 */
double mms_forcing(double x, double y, double z, void* params);

/**
 * @brief Evaluate a field at every local node of this rank. The layout matches
 * the rest of the operator: element after element (in op->element_ids order),
 * x fastest within an element.
 *
 * @param op Operator (provides n, n_elements, element_ids)
 * @param mesh Mesh
 * @param quad GLL quadrature set (reference node positions)
 * @param f Field to evaluate
 * @param params Passed through to f
 * @return Vector of size n_elements * n^3. Release with free_vector.
 */
Vector evaluate_field(const Operator* op, const Mesh* mesh, const QuadSet* quad,
                      field_fn f, void* params);

/**
 * @brief Build the right-hand side b = M f: evaluate f at the nodes, multiply
 * by the lumped mass weights, sum the contributions at shared nodes (gather-
 * scatter) and apply the Dirichlet mask. Collective.
 *
 * @return Vector of size n_elements * n^3. Release with free_vector.
 */
Vector create_rhs(const Operator* op, const Mesh* mesh, const QuadSet* quad,
                  field_fn f, void* params);

/**
 * @brief Global L2 and max-norm errors between u and the exact solution. Each
 * shared node is counted once. Collective.
 *
 * @param op Operator
 * @param u Numerical solution
 * @param u_ex Exact solution at the same nodes
 * @param l2 Output: sqrt(sum M (u-u_ex)^2)
 * @param linf Output: max |u-u_ex|
 */
void mms_error_norms(const Operator* op, const Vector* u, const Vector* u_ex,
                     double* l2, double* linf);
