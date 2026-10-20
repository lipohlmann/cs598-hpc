#!/bin/bash
# Q2 (blocked LU): build, generate cases, submit every sweep, then submit a dependent
# job that runs plot_results.py once they have all finished.
#
#   ./run_all.sh            # run from hw3/q2 on a login node
#
# Each sweep is --exclusive on its own node, so they run concurrently
# without disturbing each other's timings.  Logs: lu-sweep-<jobid>.log.
# Set PYTHON_MODULE=<name> to pick the python module the plot job loads.
# Plots (q2_*.png) land in ../report/figs, CSV in results.csv.
set -e
cd "$(dirname "$0")"

source /etc/profile >/dev/null 2>&1 || true
module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
make
python3 make_cases.py

# case file : time limit
SWEEPS=(
    "cases/q2_verify.txt:00:15:00"
    "cases/q2_bsweep.txt:01:00:00"
)
for b in 32 64 128; do
    SWEEPS+=("cases/q2_strong_b$b.txt:01:00:00")
    for tag in 1M 4M 16M; do
        SWEEPS+=("cases/q2_weak_${tag}_b$b.txt:01:00:00")
    done
done

ids=()
for s in "${SWEEPS[@]}"; do
    cases=${s%%:*}; tlim=${s#*:}
    id=$(sbatch --parsable --time="$tlim" sweep.sbatch "$cases")
    echo "submitted $cases -> job $id"
    ids+=("$id")
done

dep=$(IFS=:; echo "${ids[*]}")
plot=$(sbatch --parsable --dependency=afterany:"$dep" --export=ALL plot.sbatch \
    $(printf 'lu-sweep-%s.log ' "${ids[@]}"))
echo "submitted plot job $plot (runs after: $dep)"
echo "watch with: squeue -u \$USER ; result: lu-plot-$plot.log"
