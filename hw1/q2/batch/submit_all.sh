#!/bin/bash
# Submit every configuration Q2 asks for.  Run from the hw1/q2 directory:
#
#     ./batch/submit_all.sh          # all five
#     ./batch/submit_all.sh 2a       # just the P1 = 64-rank baseline
#
# Before the first 128-ranks-on-one-node run, check the real core count:
#
#     sinfo -p eng-instruction -o "%n %c %m"
#
# If a node has fewer than 128 cores, 128 ranks on one node needs
# --oversubscribe (and --bind-to hwthread) added to the mpirun line in
# pingpong.slurm; otherwise Open MPI will refuse to launch.

set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p data logs

# nodes:ntasks-per-node  -- 2a is the P1 baseline, the rest are 2b.
CONFIGS_2A=("1:64")
CONFIGS_2B=("1:128" "2:64" "2:96" "2:128")

case "${1:-all}" in
  2a)  CONFIGS=("${CONFIGS_2A[@]}") ;;
  2b)  CONFIGS=("${CONFIGS_2B[@]}") ;;
  all) CONFIGS=("${CONFIGS_2A[@]}" "${CONFIGS_2B[@]}") ;;
  *)   echo "usage: $0 [2a|2b|all]" >&2; exit 1 ;;
esac

for cfg in "${CONFIGS[@]}"; do
    nodes=${cfg%%:*}
    ppn=${cfg##*:}
    total=$(( nodes * ppn ))
    echo "submitting P=${total} (${nodes} node(s) x ${ppn} ranks)"
    sbatch --nodes="$nodes" \
           --ntasks-per-node="$ppn" \
           --job-name="pp_P${total}_n${nodes}" \
           batch/pingpong.slurm
done

echo
echo "watch with:  squeue -u \$USER"
echo "then:        python3 analysis/pingpong_stats.py data/pp_*.csv"
echo "and:         .venv/bin/python analysis/plot_pingpong.py data/pp_*.csv"
