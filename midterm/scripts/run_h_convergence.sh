#!/usr/bin/env bash
# h-type convergence: fixed domain and order, refine the mesh equally in all
# directions (h = L / NELEM halves at every step). The expected L2 rate for
# polynomial degree p = npts - 1 is p + 1.
#
# Avoid meshes where every GLL node lies on a zero of the manufactured
# solution: e.g. 2 elements with 3 points puts all x-nodes at multiples of
# 1/4, where sin(4 pi x) = 0, so b = 0, u = 0 and the (nodal) error is
# zero to round-off. That is why the default list starts at 3 elements.
#
# The finest run (largest NELEM, largest NPTS) also writes VTK output to
# $RESULTS_DIR/vtk/finest.pvtu.
#
#   NELEMS  list of elements per direction  (default: 3 6 12 24 48)
#   NPTS    list of GLL points per element  (default: 3 5)
#   RANKS   MPI ranks                       (default: 32)
#   TOL     CG relative tolerance           (default: 1e-14)
#   VTK     1 = write VTK for the finest run (default: 1)
# plus the settings in common.sh.
source "$(dirname "$0")/common.sh"

NELEMS=${NELEMS:-"3 6 12 24 48"}
NPTS=${NPTS:-"3 5"}
RANKS=${RANKS:-32}
TOL=${TOL:-1e-14}
VTK=${VTK:-1}

finest_n=$(echo "$NPTS" | tr ' ' '\n' | sort -n | tail -1)
finest_e=$(echo "$NELEMS" | tr ' ' '\n' | sort -n | tail -1)

for n in $NPTS; do
  for e in $NELEMS; do
    extra=()
    if [[ "$VTK" == "1" && "$n" == "$finest_n" && "$e" == "$finest_e" ]]; then
      [[ "$DRY_RUN" != "1" ]] && mkdir -p "$RESULTS_DIR/vtk"
      extra=(--vtk "$RESULTS_DIR/vtk/finest")
    fi
    submit_run h_convergence "$(printf 'npts%02d_nelem%03d' "$n" "$e")" \
      "$RANKS" --nelem "$e" --npts "$n" --tol "$TOL" --max-iters 20000 \
      "${extra[@]+"${extra[@]}"}"
  done
done
