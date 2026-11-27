//
// Adapted from the Kingfisher project.
//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Liam Pohlmann
// Copyright (c) 2026, kingfisher-project

/* The following was written heavily based on the teachings and codes found in
 * the book "Numerical Methods in Physics with Python, Second Edition" by Alex
 * Gezerlis. For more information on this text, please see
 * https://numphyspy.org/.
 */

#pragma once

#include <math.h>
#include "vector.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Tests if two double-type numbers are equal using the TOLERANCE
 * value. Taken from https://github.com/lipohlmann/hummingbird.
 *
 * @param first First number to compare
 * @param second Second number to compare
 * @return 1 if equal within 1e-15, 0 otherwise
 */
static inline int DoubleEqual(const double first, const double second) {
  double difference = second - first;
  return (fabs(difference) < 1e-15) ? 1 : 0;
}

/**
 * @brief Computes the Legendre polynomial of degree \f$n\f$ at a point \f$x\f$
 * using the recursive formula:
 *
 * \f[
 * P_{n+1}(x)=\frac{(2n+1)xP_{n}(x)-nP_{n-1}(x)}{n+1}
 * \f]
 *
 * with:
 *
 * \f[
 * P_0=1,\quad P_1=x.
 * \f]
 *
 * The above is implemented to evaluate the n-th order Legendre polynomial by
 * building from the bottom up, as opposed to using a recursion relation. This
 * greatly improves the evaluation speed. This algorithm was taken from
 * https://github.com/CambridgeUniversityPress/NumericalMethodsPhysicsWithPython/blob/master/second_edition/codes/legendre.py
 * written by Alex Gezerlis for his book, "Numerical Methods in Physics With
 * Python, Second Edition".
 *
 *  Legendre polynomials are defined on the interval \f$x\in
 * [-1,1]\f$.
 *
 * @param n Order of polynomial to evaluate
 * @param x Point to evaluate at
 * @return double
 */
double LegendrePolynomial(const int n, const double x);

/**
 * @brief Computes the first derivative of the Legendre polynomial of degree
 * \f$n\f$ at a point \f$x\f$ using the recurrence relation:
 *
 * \f[
 * P'_n (x)=\frac{nP_{n-1}(x)-nxP_n(x)}{1-x^2}
 * \f]
 *
 * The value of the derivative at the bounds is special:
 *
 * \f[
 * P'_n(x)=\begin{cases} (-1)^{n-1}\frac{n(n+1)}{2} & x=-1
 * \\ \frac{n(n+1)}{2} &x=1 \end{cases}.
 * \f]
 *
 * @param n Order of polynomial
 * @param x Point to evaluate at
 * @return double
 */
double LegendrePolynomialPrime(const int n, const double x);

/**
 * @brief Computes the second derivative of the Legendre polynomial of degree
 * \f$n\f$ using Legendre's differential equation:
 *
 * \f[
 * (1-x^2)P_n''(x)-2xP_n'(x)+n(n+1)P_n(x)=0.
 * \f]
 *
 * The second derivative at the boundaries is special:
 *
 * \f[
 * P''_n(x)=\begin{cases} (-1)^{n}\frac{(n-1)n(n+1)(n+2)}{8} & x=-1
 * \\ \frac{(n-1)n(n+1)(n+2)}{8} & x=1 \end{cases}.
 * \f]
 *
 * @param n Order
 * @param x Location to evaluate at
 * @return double
 */
double LegendrePolynomialPrimePrime(const int n, const double x);

/**
 * @brief Compute all roots of the Legendre polynomial of order \f$n\f$.
 *
 * Roots are sorted ascending. For odd n the middle root (0) relies on
 * create_vector zero-initializing. The caller must release the result with
 * free_vector.
 *
 * @param n Order
 * @return Vector of n roots
 */
Vector AllLegendreRoots(const int n);

/**
 * @brief Computes the k-th root of the n-th order Legendre polynomial by first
 * approximating the root with ApproximateLegendreRoot, then using Newton's
 * method to converge to the root.
 *
 * @param n Order of the polynomial
 * @param k Root to find
 * @return double
 */
double LegendreRoot(const int n, const int k);

/**
 * @brief Approximates the \f$k\f$-th root of the \f$n\f$-th order Legendre
 * polynomial with:
 *
 * \f[
 * x_k^{(0)}\approx \cos \left(\frac{4k+3}{4n+2}\pi\right).
 * \f]
 *
 * @param n Order
 * @param k Root number
 * @return double
 */
double ApproximateLegendreRoot(const int n, const int k);

/**
 * @brief Computes all roots of the \f$P'_n(x)\f$ polynomial. A total of
 * \f$n-1\f$ roots will be computed and returned, sorted ascending. For even n
 * the middle root (0) relies on create_vector zero-initializing. The caller
 * must release the result with free_vector.
 *
 * @param n Order
 * @return Vector of n-1 roots
 */
Vector AllLegendrePrimeRoots(const int n);

/**
 * @brief Computes the \f$k\f$-th root of the \f$n\f$-th order first derivative
 * of the Legendre polynomial by first approximating with
 * ApproximateLegendrePrimeRoot, then using Newton's method to converge
 *
 * @param n Order
 * @param k Root number
 * @return double
 */
double LegendrePrimeRoot(const int n, const int k);

/**
 * @brief Approximates the \f$k\f$-th root of the first derivative of the
 * \f$n\f$-th degree Legendre polynomial using the average of the approximations
 * (ApproximateLegendreRoot()) of the surrounding roots of the \f$n\f$-th degree
 * Legendre polynomial. Note that zero-indexing is used, so the roots of the
 * \f$P'_4 (x)\f$ polynomial are \f$k=0,1,2\f$. The approximations of these are
 * given by (negated so that k=0 is the most negative root):
 *
 * \f[
 * x_k\approx -\frac{1}{2} \left( \cos \left(\frac{4k+3}{4n+2}\pi\right) + \cos
 * \left(\frac{4(k+1)+3}{4n+2}\pi\right) \right).
 * \f]
 *
 * @param n Order
 * @param k Root number (zero-indexed)
 * @return double
 */
double ApproximateLegendrePrimeRoot(const int n, const int k);
