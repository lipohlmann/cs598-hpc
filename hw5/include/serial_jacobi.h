#pragma once

#include "matrix.h"
#include "vector.h"

Vector SerialJacobi(const Matrix* A, const Vector* b, const double tol);
