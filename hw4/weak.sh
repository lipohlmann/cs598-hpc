#!/bin/bash
# Weak scaling: fixed rows per rank, so N = P * NLOCAL grows with the rank count.
#
#   ./weak.sh
#   SOLVER=thomas ./weak.sh          # the textbook solver
#   NLOCAL=262144 PS="1 2 4 8" CSV=results/weak_big.csv TIME=01:00:00 ./weak.sh
#
# Results are appended to $CSV, the job log goes to weak-<jobid>.log.

source "$(dirname "$0")/scaling_common.sh"

NLOCAL=${NLOCAL:-65536}    # 2^16
CSV=${CSV:-results/weak.csv}

runs=()
for p in $PS; do runs+=("$p $((p * NLOCAL))"); done
submit_sweep weak "$CSV" "${runs[@]}"
