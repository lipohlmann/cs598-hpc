#!/usr/bin/env bash
# Run or submit every study. All runs are independent Slurm jobs.
set -euo pipefail
dir="$(dirname "$0")"
"$dir/run_p_convergence.sh"
"$dir/run_h_convergence.sh"
"$dir/run_strong_scaling.sh"
"$dir/run_weak_scaling.sh"
