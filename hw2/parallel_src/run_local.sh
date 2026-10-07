#!/bin/bash
# Quick MPI runs on a login node (small P, small N only -- be polite).
# The login nodes have no InfiniBand, so the module's default UCX
# transport fails; force the ob1 PML over shared memory instead.
#
#   ./run_local.sh 4 ./test_poisson 128 512 2 1 pairwise

if [ $# -lt 2 ]; then
    echo "usage: $0 nranks executable [args...]" >&2
    exit 1
fi
P=$1; shift

source /etc/profile >/dev/null 2>&1
module load openmpi
export OMPI_MCA_ras_base_verbose=0
export OMPI_MCA_pml=ob1
export OMPI_MCA_btl=self,sm

exec mpirun --oversubscribe -np "$P" "$@"
