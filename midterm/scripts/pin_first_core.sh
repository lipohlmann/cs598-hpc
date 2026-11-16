#!/usr/bin/env bash
# Pin the calling task to the first core of the CPU set the launcher gave it,
# then exec the command. With `srun --cpus-per-task C` every rank gets a block
# of C cores; pinning to the first one leaves the ranks spread evenly over the
# node (C - 1 idle cores between neighbours) and stops them from migrating.
#   usage: pin_first_core.sh COMMAND [ARGS...]
set -euo pipefail
cpus=$(taskset -cp $$)
cpus=${cpus##*: }
exec taskset -c "${cpus%%[,-]*}" "$@"
