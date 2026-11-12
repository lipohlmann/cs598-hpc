#!/usr/bin/env bash
# p-type convergence: fixed mesh, increase the GLL points per element.
# Spectral convergence shows up as a straight line of log(error) vs npts.
#
#   NELEM   elements per direction          (default: 4)
#   NPTS    list of GLL points per element  (default: 3 4 ... 16)
#   RANKS   MPI ranks                       (default: 4)
#   TOL     CG relative tolerance           (default: 1e-14)
# plus the settings in common.sh.
source "$(dirname "$0")/common.sh"

NELEM=${NELEM:-4}
NPTS=${NPTS:-"3 4 5 6 7 8 9 10 11 12 13 14 15 16"}
RANKS=${RANKS:-4}
TOL=${TOL:-1e-14}

for n in $NPTS; do
  submit_run p_convergence "$(printf 'npts%02d' "$n")" "$RANKS" \
    --nelem "$NELEM" --npts "$n" --tol "$TOL" --max-iters 5000
done
