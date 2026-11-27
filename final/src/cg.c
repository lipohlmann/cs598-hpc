#include "cg.h"

#include <math.h>
#include <stddef.h>

// y += alpha * x
static void axpy(Vector* y, const double alpha, const Vector* x) {
  for (size_t i = 0; i < y->size; i++) y->data[i] += alpha * x->data[i];
}

// p = r + beta * p
static void xpby(Vector* p, const Vector* r, const double beta) {
  for (size_t i = 0; i < p->size; i++)
    p->data[i] = r->data[i] + beta * p->data[i];
}

/*
Matlab version of serial CG. 

function x = conjugate_gradient(A, b, x0, tol)
% Return the solution to `A * x = b` using the conjugate gradient method.
% Reminder: A should be symmetric and positive definite.

    if nargin < 4
        tol = eps;
    end

    r = b - A * x0;
    p = r;
    rsold = r' * r;

    x = x0;

    while sqrt(rsold) > tol
        Ap = A * p;
        alpha = rsold / (p' * Ap);
        x = x + alpha * p;
        r = r - alpha * Ap;
        rsnew = r' * r;
        p = r + (rsnew / rsold) * p;
        rsold = rsnew;
    end
end
*/


int conjugate_gradient(Vector* x, const Vector* b, const double rel_tol,
                       const int max_iters, void* ctx) {
  const size_t n = b->size;
  Vector r = create_vector(n);
  Vector p = create_vector(n);
  Vector Ap = create_vector(n);

  // r = b - A x
  apply_A(&Ap, x, ctx);
  for (size_t i = 0; i < n; i++) r.data[i] = b->data[i] - Ap.data[i];

  // p = r
  for (size_t i = 0; i < n; i++) p.data[i] = r.data[i];

  double rr = global_dot(&r, &r, ctx);
  const double b_norm = sqrt(global_dot(b, b, ctx));
  const double threshold = rel_tol * (b_norm > 0.0 ? b_norm : 1.0);

  int iters = -1;
  for (int it = 0; it < max_iters; it++) {
    if (sqrt(rr) < threshold) {
      iters = it;
      break;
    }

    apply_A(&Ap, &p, ctx);

    const double alpha = rr / global_dot(&p, &Ap, ctx);
    axpy(x, alpha, &p);    // x += alpha * p
    axpy(&r, -alpha, &Ap); // r -= alpha * A p

    const double rr_new = global_dot(&r, &r, ctx);
    xpby(&p, &r, rr_new / rr);  // p = r + beta * p
    rr = rr_new;
  }
  if (iters < 0 && sqrt(rr) < threshold) iters = max_iters;

  free_vector(&r);
  free_vector(&p);
  free_vector(&Ap);
  return iters;
}

