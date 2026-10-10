#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=00:10:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=1
#SBATCH --job-name=pingpong
#SBATCH --output=pingpong-%j.log

# Point-to-point ping-pong, once with both ranks on one node and once
# with one rank on each of two nodes.  Writes pingpong_intranode.csv and
# pingpong_internode.csv to the output directory.
#
#   sbatch pingpong.sbatch                 # results/
#   sbatch pingpong.sbatch my_results
#   MAX_BYTES=1048576 sbatch pingpong.sbatch

OUTPUT_DIR=${1:-results}
MAX_BYTES=${MAX_BYTES:-134217728}

module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log

if [ ! -x ./pingpong ]; then
    echo "./pingpong not found -- run 'make' before sbatch" >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"

echo "node $(hostname): $(lscpu | grep 'Model name' | sed 's/.*: *//')"

echo "=== intranode ==="
mpirun -np 2 --map-by core --bind-to core --report-bindings \
    ./pingpong "$MAX_BYTES" "$OUTPUT_DIR" intranode

echo "=== internode ==="
mpirun -np 2 --map-by node --bind-to core --report-bindings \
    ./pingpong "$MAX_BYTES" "$OUTPUT_DIR" internode
