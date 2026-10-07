#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction    # <- replace with your partition
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --job-name=poisson
#SBATCH --output=poisson-%j.log

# Rank count comes from the allocation; arguments go to test_poisson.
# Override the size on the sbatch command line, e.g.
#
#   sbatch poisson.sbatch                                    # P=1, defaults
#   sbatch --ntasks-per-node=16 poisson.sbatch 2048 2048 1 1
#   sbatch --nodes=2 --ntasks-per-node=64 poisson.sbatch 2048 2048 1 1 pairwise
#
# test_poisson args:  [Nx] [Ny] [Lx] [Ly] [allatonce|pairwise]
# eng-instruction CPU nodes have 128 cores each.

module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log

if [ ! -x ./test_poisson ]; then
    echo "./test_poisson not found -- run 'make' (or ./driver) before sbatch" >&2
    exit 1
fi

mpirun -np "$SLURM_NTASKS" --map-by core --bind-to core ./test_poisson "$@"
