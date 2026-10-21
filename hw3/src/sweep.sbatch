#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --constraint=amd               # AMD EPYC 7713 nodes (128 cores)
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --exclusive                    # nobody else on the node while timing
#SBATCH --job-name=lu-sweep
#SBATCH --output=lu-sweep-%j.log

# Run a list of cases from a text file, one per line:
#
#   <nranks> <executable> [args...]
#
# Blank lines and lines starting with # are skipped.  INPUT=... on a
# line's front (e.g. "INPUT=32 16 ./factor_parallel 4000 4001") is fed
# on stdin for that case.  The allocation must hold the largest nranks.
#
#   sbatch sweep.sbatch cases/strong.txt
#   sbatch --nodes=2 sweep.sbatch cases/weak.txt       # up to 256 ranks
#
# Every case's output goes to the job log, preceded by a "=== ... ==="
# header line, so it is easy to grep/parse afterwards.

CASES=${1:?usage: sbatch sweep.sbatch casefile}

module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log
export OMP_NUM_THREADS=1

if [ ! -r "$CASES" ]; then
    echo "case file $CASES not found" >&2
    exit 1
fi

echo "cases: $CASES"
echo "node(s) $SLURM_JOB_NODELIST: $(lscpu | grep 'Model name' | sed 's/.*: *//'), SLURM_NTASKS=$SLURM_NTASKS"

# Which process binding works depends on how Slurm hands out cpus (whole
# cores or hardware threads; --exclusive changes it), so use the first
# of these that can launch both one rank and the full allocation.
# spread.sh comes first: it pins rank r to a core on CCD r % 16 (8 cores
# sharing one L3 and one memory link), so small rank counts are not all
# squeezed onto one CCD's memory bandwidth.  (--map-by l3cache is refused
# in these allocations, and pe-lists depend on each node's numbering.)
export SPREAD_CPUS=$(./spread.sh)
echo "rank -> cpu order: $SPREAD_CPUS"
SPREAD="mpirun --bind-to none -x SPREAD_CPUS"
MPIRUN=""
PIN=""        # ./spread.sh with $SPREAD; goes after -np, before the program
for candidate in \
    "$SPREAD" \
    "mpirun --map-by core --bind-to core" \
    "mpirun --map-by hwthread --bind-to hwthread" \
    "mpirun --bind-to none"; do
    pin=""; [ "$candidate" = "$SPREAD" ] && pin=./spread.sh
    if $candidate -np 1 $pin true >/dev/null 2>&1 &&
       $candidate -np "$SLURM_NTASKS" $pin true >/dev/null 2>&1; then
        MPIRUN=$candidate
        PIN=$pin
        break
    fi
done
if [ -z "$MPIRUN" ]; then
    echo "no mpirun binding option could launch $SLURM_NTASKS ranks; the error was:" >&2
    mpirun --bind-to none -np "$SLURM_NTASKS" true
    exit 1
fi
if [ -z "$PIN" ] && [ -z "$ALLOW_PACKED" ]; then
    echo "spread.sh could not launch; refusing to time with packed ranks" >&2
    echo "(set ALLOW_PACKED=1 to run anyway)" >&2
    exit 1
fi
echo "launching with: $MPIRUN -np <n> $PIN"
# record where ranks really land (each prints its cpu affinity)
$MPIRUN -np 4 $PIN grep Cpus_allowed_list /proc/self/status

while read -r line <&3; do
    case "$line" in ''|\#*) continue ;; esac
    input=""
    if [[ $line == INPUT=* ]]; then
        input=${line%% *}; input=${input#INPUT=}
        line=${line#* }
    fi
    read -r np cmd <<< "$line"
    if [ "$np" -gt "$SLURM_NTASKS" ]; then
        echo "=== SKIP np=$np > SLURM_NTASKS=$SLURM_NTASKS: $cmd ==="
        continue
    fi
    echo "=== np=$np input=$input cmd=$cmd ==="
    # shellcheck disable=SC2086   # word-split cmd into program + args
    if [ -n "$input" ]; then
        echo "$input" | $MPIRUN -np "$np" $PIN $cmd
    else
        $MPIRUN -np "$np" $PIN $cmd </dev/null
    fi
    echo "=== exit $? ==="
done 3< "$CASES"
