#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  size_t ns[4] = {8, 16, 32, 64};

  for (int i = 0; i < 4; i++) {
    tridiag_matrix matrix;
    matrix.n = ns[i];

    random_spd_tridiag(&matrix);

    // solve
  }

  return 0;
}
