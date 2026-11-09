# Shared by strong.sh and weak.sh; not meant to be run on its own.
#
# Builds build/scaling and submits ONE job that runs every (P, N) pair in turn.
# One allocation means every rank count runs on the same hardware, with the
# node to ourselves. The partition mixes 64-core and 128-core nodes;
# --ntasks-per-node=128 keeps us on the 128-core ones.

set -e
cd "$(dirname "${BASH_SOURCE[0]}")"

CORES_PER_NODE=128
PS=${PS:-1 2 4 8 16 32 64 128 256}
TIME=${TIME:-01:30:00}
SOLVER=${SOLVER:-dd}    # dd or thomas

# usage: submit_sweep job-name results.csv "P N" "P N" ...
submit_sweep() {
    local name=$1 csv=$2
    shift 2

    type module >/dev/null 2>&1 || source /usr/share/lmod/lmod/init/bash
    module load openmpi

    # The other files in src/ are the library; skip any that bring their own main.
    local libs=() f
    for f in src/*.cc; do
        [ "$f" = src/scaling.cc ] && continue
        grep -qE '\bint[[:space:]]+main[[:space:]]*\(' "$f" && continue
        libs+=("$f")
    done

    # Build to a temp name and rename, so a job that is already running or
    # about to launch never sees a half-written binary.
    mkdir -p build "$(dirname "$csv")"
    mpicxx -O2 -Wall -Wextra -std=c++17 -Iinclude src/scaling.cc "${libs[@]}" \
        -o "build/scaling.$$" -lm
    mv "build/scaling.$$" build/scaling

    local max_p=0 cmds="" run p n
    for run in "$@"; do
        read -r p n <<<"$run"
        [ "$p" -gt "$max_p" ] && max_p=$p
        cmds+="mpirun -np $p --map-by core --bind-to core build/scaling $n $csv $SOLVER </dev/null || echo \"FAILED: P=$p N=$n\""$'\n'
    done
    local nodes=$(((max_p + CORES_PER_NODE - 1) / CORES_PER_NODE))

    [ -s "$csv" ] && echo "note: $csv already has data; new rows will be appended"
    echo "$name ($SOLVER): ${#@} runs, P up to $max_p on $nodes node(s) -> $csv"

    # --mem=0 because every rank holds full-length arrays (~80 bytes per row
    # per rank), far more than the default per-CPU memory at large N and P
    sbatch <<SCRIPT
#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=$TIME
#SBATCH --nodes=$nodes
#SBATCH --ntasks-per-node=$CORES_PER_NODE
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH --mem=0
#SBATCH --job-name=$name
#SBATCH --output=$name-%j.log

module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log
export OMP_NUM_THREADS=1

echo "nodes: \$SLURM_JOB_NODELIST"
$cmds
SCRIPT
}
