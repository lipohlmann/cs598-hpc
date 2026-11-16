#!/bin/bash
# Verification: solve a few problem sizes on a few rank counts and record the
# residual ||f - A x|| of each, to show the solver really solves the system.
#
#   ./verify.sh
#   SOLVER=thomas ./verify.sh        # the textbook solver (P must be a power of 2)
#   NS="1000 4194304" PS="1 3 7" CSV=results/verify_odd.csv ./verify.sh
#
# Results are appended to $CSV, the job log goes to verify-<jobid>.log.

PROG=verify
PS=${PS:-1 2 4 16 64}
TIME=${TIME:-00:10:00}
source "$(dirname "$0")/scaling_common.sh"

# small sizes matter: the solver forgets a bad start value over many rows
NS=${NS:-128 1000 65536 1048576}
CSV=${CSV:-results/verify.csv}

runs=()
for n in $NS; do
    for p in $PS; do runs+=("$p $n"); done
done
submit_sweep verify "$CSV" "${runs[@]}"
