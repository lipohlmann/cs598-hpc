#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --job-name=fst-sweep
#SBATCH --output=fst-sweep-%j.log

# Single-core performance of the batched FST over (m, n): transform
# length n = N - 1 for every N in GRID_SIZES, batch size m = 1, 2, 4,
# ..., N, which covers the local slab sizes N/P of the parallel solver.
# Appends one row per (m, n) to fst.csv in the output directory.
# The largest case (m = 8192, n = 8191) needs about 5 GB, hence --mem.
#
#   sbatch fst_sweep.sbatch                 # results/
#   sbatch fst_sweep.sbatch my_results
#   GRID_SIZES="2048 8192" sbatch fst_sweep.sbatch

OUTPUT_DIR=${1:-results}
GRID_SIZES=${GRID_SIZES:-"32 128 512 2048 8192"}
# Entries transformed per timing run, split into reps of m*n entries.
ENTRIES_PER_RUN=${ENTRIES_PER_RUN:-400000000}

module load openmpi   # test_block_fst is serial but links the MPI library

if [ ! -x ./test_block_fst ]; then
    echo "./test_block_fst not found -- run 'make' before sbatch" >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"
rm -f "$OUTPUT_DIR/fst.csv"   # rows are appended, so start a fresh table

for grid_size in $GRID_SIZES; do
    transform_length=$((grid_size - 1))
    batch_size=1
    while [ "$batch_size" -le "$grid_size" ]; do
        num_reps=$((ENTRIES_PER_RUN / (batch_size * transform_length)))
        [ "$num_reps" -gt 2000 ] && num_reps=2000
        [ "$num_reps" -lt 5 ] && num_reps=5
        echo "=== m=$batch_size n=$transform_length nrep=$num_reps ==="
        ./test_block_fst "$batch_size" "$transform_length" "$num_reps" "$OUTPUT_DIR"
        batch_size=$((batch_size * 2))
    done
done
