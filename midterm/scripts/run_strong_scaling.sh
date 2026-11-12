#!/usr/bin/env bash
# Strong scaling: fixed global problem, increase the number of ranks.
# Speedup S(P) = T(1) / T(P), efficiency E(P) = S(P) / P.
#
#   NELEM      elements per direction        (default: 32)
#   NPTS       GLL points per element        (default: 6)
#   RANKS      list of rank counts           (default: powers of 2 up to MAX_RANKS)
#   MAX_RANKS  largest rank count            (default: 256)
#   MAX_ITERS  CG iteration cap; a fixed cap keeps every run doing the same
#              work (default: 200)
# plus the settings in common.sh (REPS repetitions per rank count).
source "$(dirname "$0")/common.sh"

NELEM=${NELEM:-32}
NPTS=${NPTS:-6}
MAX_RANKS=${MAX_RANKS:-256}
RANKS=${RANKS:-$(powers_of_two_up_to "$MAX_RANKS")}
MAX_ITERS=${MAX_ITERS:-200}

for p in $RANKS; do
  for rep in $(seq 1 "$REPS"); do
    submit_run strong_scaling "$(printf 'ranks%05d_rep%d' "$p" "$rep")" "$p" \
      --nelem "$NELEM" --npts "$NPTS" --max-iters "$MAX_ITERS"
  done
done
