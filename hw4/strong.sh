#!/bin/bash
# Strong scaling: fixed problem size, growing rank count.
#
#   ./strong.sh
#   SOLVER=thomas ./strong.sh        # the textbook solver
#   N=67108864 PS="1 2 4 8" CSV=results/strong_big.csv TIME=01:00:00 ./strong.sh
#
# Results are appended to $CSV, the job log goes to strong-<jobid>.log.

source "$(dirname "$0")/scaling_common.sh"

N=${N:-16777216}    # 2^24
CSV=${CSV:-results/strong.csv}

runs=()
for p in $PS; do runs+=("$p $N"); done
submit_sweep strong "$CSV" "${runs[@]}"
