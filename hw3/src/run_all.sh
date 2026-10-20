#!/bin/bash
# Build, generate cases, submit every Q1 sweep, then submit a dependent job
# that runs plot_results.py once they have all finished.
#
#   ./run_all.sh            # run from hw3/src on a login node
#
# Each sweep is --exclusive on its own node, so they run concurrently
# without disturbing each other's timings.  Logs: lu-sweep-<jobid>.log.
# Plots/CSV land in ../report/figs and results.csv.
set -e
cd "$(dirname "$0")"

source /etc/profile >/dev/null 2>&1 || true
module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
make
python3 make_cases.py

# case file : time limit
SWEEPS=(
    "cases/verify.txt:00:15:00"
    "cases/strong.txt:01:00:00"
    "cases/weak_250k.txt:01:00:00"
    "cases/weak_1M.txt:01:00:00"
    "cases/weak_4M.txt:02:00:00"
)

ids=()        # all jobs (the plot job waits for every one)
plot_ids=()   # jobs whose logs are plotted (verify runs are not)
for s in "${SWEEPS[@]}"; do
    cases=${s%%:*}; tlim=${s#*:}
    id=$(sbatch --parsable --time="$tlim" sweep.sbatch "$cases")
    echo "submitted $cases -> job $id"
    ids+=("$id")
    case "$cases" in *verify.txt) ;; *) plot_ids+=("$id") ;; esac
done

dep=$(IFS=:; echo "${ids[*]}")
plot=$(sbatch --parsable --dependency=afterany:"$dep" --export=ALL plot.sbatch \
    $(printf 'lu-sweep-%s.log ' "${plot_ids[@]}"))
echo "submitted plot job $plot (runs after: $dep)"
echo "watch with: squeue -u \$USER ; result: lu-plot-$plot.log"
