#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --constraint=amd               # AMD EPYC 7713 nodes (128 cores); BLIS is tuned for these
#SBATCH --time=00:15:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=1
#SBATCH --job-name=lu
#SBATCH --output=lu-%j.log

# One run of any program in this directory.  Rank count comes from the
# allocation; everything after the program name is passed through.
#
#   sbatch run.sbatch ./factor_parallel 2000 2001
#   sbatch --ntasks-per-node=64 run.sbatch ./factor_parallel 8000 8001
#   INPUT=32 sbatch run.sbatch ./factor_parallel 4000 4001    # "32" on stdin
#   sbatch --nodes=2 --ntasks-per-node=128 run.sbatch ./factor_parallel ...

if [ $# -lt 1 ]; then
    echo "usage: [INPUT=...] sbatch run.sbatch executable [args...]" >&2
    exit 1
fi

module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log
export OMP_NUM_THREADS=1

if [ ! -x "$1" ]; then
    echo "$1 not found -- run 'make' before sbatch" >&2
    exit 1
fi

echo "node $(hostname): $(lscpu | grep 'Model name' | sed 's/.*: *//'), SLURM_NTASKS=$SLURM_NTASKS"
echo "cmd: $*"

# Which process binding works depends on how Slurm hands out cpus (whole
# cores or hardware threads; --exclusive changes it), so use the first
# of these that can launch both one rank and the full allocation.
MPIRUN=""
for candidate in \
    "mpirun --map-by core --bind-to core" \
    "mpirun --map-by hwthread --bind-to hwthread" \
    "mpirun --bind-to none"; do
    if $candidate -np 1 true >/dev/null 2>&1 &&
       $candidate -np "$SLURM_NTASKS" true >/dev/null 2>&1; then
        MPIRUN=$candidate
        break
    fi
done
if [ -z "$MPIRUN" ]; then
    echo "no mpirun binding option could launch $SLURM_NTASKS ranks; the error was:" >&2
    mpirun --bind-to none -np "$SLURM_NTASKS" true
    exit 1
fi
echo "launching with: $MPIRUN"

if [ -n "$INPUT" ]; then
    echo "$INPUT" | $MPIRUN -np "$SLURM_NTASKS" "$@"
else
    $MPIRUN -np "$SLURM_NTASKS" "$@" </dev/null
fi
