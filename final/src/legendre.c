#include "legendre.h"

#include <math.h>

double LegendrePolynomial(const int n, const double x) {
  switch (n) {
    case 0:
      return 1.0;

    case 1:
      return x;

    default: {
      double previous = 1.0;
      double current = x;
      for (int j = 1; j < n; j++) {
        double next =
            ((2.0 * (double)j + 1.0) * x * current - (double)(j)*previous) /
            ((double)(j) + 1.0);
        previous = current;
        current = next;
      }
      return current;
    }
  }
}

double LegendrePolynomialPrime(const int n, const double x) {
  if (DoubleEqual(x, -1.0) == 1)
    return pow(-1, n - 1) * (double)n * (n + 1) / 2.0;
  else if (DoubleEqual(x, 1.0) == 1)
    return (double)n * (n + 1) / 2.0;

  switch (n) {
    case 0:
      return 0;

    case 1:
      return 1.0;

    default:
      return ((double)(n)*LegendrePolynomial(n - 1, x) -
              (double)(n)*x * LegendrePolynomial(n, x)) /
             (1.0 - x * x);
  }
}

double LegendrePolynomialPrimePrime(const int n, const double x) {
  if (DoubleEqual(x, 1.0) == 1)
    return (double)n * (n - 1) * (n + 1) * (n + 2) / 8.0;
  else if (DoubleEqual(x, -1.0) == 1)
    return pow(-1.0, n) * (double)n * (n - 1) * (n + 1) * (n + 2) / 8.0;

  return (2 * x * LegendrePolynomialPrime(n, x) -
          (double)n * (n + 1) * LegendrePolynomial(n, x)) /
         (1.0 - x * x);
}

Vector AllLegendreRoots(const int n) {
  if (n < 1) return create_vector(0);
  const int n_even = n / 2;
  Vector computed_roots = create_vector(n);

  for (int i = 0; i < n_even; i++) {
    computed_roots.data[n - 1 - i] = LegendreRoot(n, i);
    computed_roots.data[i] = -computed_roots.data[n - 1 - i];
  }
  return computed_roots;
}

double LegendreRoot(const int n, const int k) {
  double x_old = ApproximateLegendreRoot(n, k);
  int safety = 10000;
  for (int i = 0; i < safety; i++) {
    double x_new = x_old - LegendrePolynomial(n, x_old) /
                               LegendrePolynomialPrime(n, x_old);
    double relative_error = fabs((x_new - x_old) / fmax(1.0, fabs(x_new)));

    if (relative_error < 1e-12) return x_new;
    x_old = x_new;
  }
  return NAN;  // did not converge
}

double ApproximateLegendreRoot(const int n, const int k) {
  return cos((4.0 * (double)(k) + 3.0) / (4.0 * (double)(n) + 2.0) * M_PI);
}

Vector AllLegendrePrimeRoots(const int n) {
  if (n < 2) return create_vector(0);
  const int n_even = (n - 1) / 2;
  Vector computed_roots = create_vector(n - 1);

  for (int i = 0; i < n_even; i++) {
    computed_roots.data[i] = LegendrePrimeRoot(n, i);
    computed_roots.data[n - 2 - i] = -computed_roots.data[i];
  }
  return computed_roots;
}

double LegendrePrimeRoot(const int n, const int k) {
  double x_old = ApproximateLegendrePrimeRoot(n, k);
  int safety = 100;

  for (int i = 0; i < safety; i++) {
    double x_new = x_old - LegendrePolynomialPrime(n, x_old) /
                               LegendrePolynomialPrimePrime(n, x_old);
    double relative_error = fabs((x_new - x_old) / fmax(1.0, fabs(x_new)));

    if (relative_error < 1e-12) return x_new;
    x_old = x_new;
  }
  return NAN;  // did not converge
}

double ApproximateLegendrePrimeRoot(const int n, const int k) {
  return -(ApproximateLegendreRoot(n, k) + ApproximateLegendreRoot(n, k + 1)) /
         2.0;
}