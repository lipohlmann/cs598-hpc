#!/usr/bin/env bash
# Weak scaling: fixed work per rank. Each rank owns a BLOCK^3 block of
# elements; for P = 2^k ranks the k doublings of the mesh are spread over x, y,
# z in turn (x first). The domain grows with the mesh (L = n_elements / BLOCK),
# so the element size h is the same in every run and every rank does the same
# local work. Efficiency E(P) = T(1) / T(P).
#
# The unpreconditioned CG iteration count grows with the problem size, so
# compare the time per iteration as well as the total time (analyze.py
# reports both). MAX_ITERS fixes the iteration count to compare equal work.
#
#   BLOCK      elements per direction per rank (default: 8, i.e. 512/rank)
#   NPTS       GLL points per element          (default: 6)
#   RANKS      list of rank counts, powers of 2 (default: up to MAX_RANKS)
#   MAX_RANKS  largest rank count              (default: 256)
#   MAX_ITERS  CG iteration cap                (default: 200)
# plus the settings in common.sh (REPS repetitions per rank count).
source "$(dirname "$0")/common.sh"

BLOCK=${BLOCK:-8}
NPTS=${NPTS:-6}
MAX_RANKS=${MAX_RANKS:-256}
RANKS=${RANKS:-$(powers_of_two_up_to "$MAX_RANKS")}
MAX_ITERS=${MAX_ITERS:-200}

for p in $RANKS; do
  # k = log2(p); fails for non powers of two
  k=0
  while (((1 << k) < p)); do k=$((k + 1)); done
  if (((1 << k) != p)); then
    echo "error: weak scaling needs power-of-two rank counts (got $p)" >&2
    exit 1
  fi
  ax=$(((k + 2) / 3)) ay=$(((k + 1) / 3)) az=$((k / 3))
  nx=$((BLOCK << ax)) ny=$((BLOCK << ay)) nz=$((BLOCK << az))
  lx=$((1 << ax)) ly=$((1 << ay)) lz=$((1 << az))  # = n / BLOCK

  for rep in $(seq 1 "$REPS"); do
    submit_run weak_scaling "$(printf 'ranks%05d_rep%d' "$p" "$rep")" "$p" \
      --nx "$nx" --ny "$ny" --nz "$nz" --lx "$lx" --ly "$ly" --lz "$lz" \
      --npts "$NPTS" --max-iters "$MAX_ITERS"
  done
done
