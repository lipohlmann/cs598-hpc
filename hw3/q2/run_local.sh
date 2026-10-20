#!/bin/bash
# Quick MPI runs on a login node (small P, small n only -- be polite).
# The login nodes have no InfiniBand, so the module's default UCX
# transport fails; force the ob1 PML over shared memory instead.
#
#   ./run_local.sh 4 ./factor_blocked 200 201
#   INPUT=8 ./run_local.sh 4 ./factor_blocked 200 201   # "8" on stdin (rank 0)

if [ $# -lt 2 ]; then
    echo "usage: [INPUT=...] $0 nranks executable [args...]" >&2
    exit 1
fi
P=$1; shift

source /etc/profile >/dev/null 2>&1
module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0 >/dev/null 2>&1
export OMPI_MCA_ras_base_verbose=0
export OMPI_MCA_pml=ob1
export OMPI_MCA_btl=self,sm
export OMP_NUM_THREADS=1

if [ -n "$INPUT" ]; then
    echo "$INPUT" | mpirun --oversubscribe -np "$P" "$@"
else
    mpirun --oversubscribe -np "$P" "$@" </dev/null
fi
