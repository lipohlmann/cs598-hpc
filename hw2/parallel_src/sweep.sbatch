#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=00:30:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=128
#SBATCH --cpus-per-task=1
#SBATCH --job-name=poisson-sweep
#SBATCH --output=poisson-sweep-%j.log

# Parallel scaling sweep of test_poisson: every grid size in GRID_SIZES,
# every power-of-two rank count that fits in the allocation (and in the
# grid), both transpose modes.  One timing CSV per run lands in the
# output directory; the largest-P all-at-once run of each grid size also
# writes the sampled solution.
#
#   sbatch sweep.sbatch                          # results/, 1 node, P <= 128
#   sbatch sweep.sbatch my_results
#   sbatch --nodes=2 sweep.sbatch                # P up to 256
#   GRID_SIZES="2048 8192" sbatch sweep.sbatch   # override the grid sizes
#
# Grid sizes are N = Nx = Ny and must satisfy 2N = 4^k: 32, 128, 512,
# 2048, 8192.  N = 8192 needs several GB at small P, so it is not in the
# default list.

OUTPUT_DIR=${1:-results}
GRID_SIZES=${GRID_SIZES:-"32 128 512 2048"}
NUM_REPS=${NUM_REPS:-5}
SOLUTION_SAMPLE_INTERVALS=${SOLUTION_SAMPLE_INTERVALS:-128}

module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log

if [ ! -x ./test_poisson ]; then
    echo "./test_poisson not found -- run 'make' before sbatch" >&2
    exit 1
fi
mkdir -p "$OUTPUT_DIR"

for grid_size in $GRID_SIZES; do
    # Rank counts: powers of two up to the allocation, with P <= N - 1.
    rank_counts=""
    num_ranks=1
    while [ "$num_ranks" -le "$SLURM_NTASKS" ] && [ "$num_ranks" -lt "$grid_size" ]; do
        rank_counts="$rank_counts $num_ranks"
        largest_rank_count=$num_ranks
        num_ranks=$((num_ranks * 2))
    done

    for num_ranks in $rank_counts; do
        for transpose_mode in allatonce pairwise; do
            sample_intervals=0
            if [ "$num_ranks" -eq "$largest_rank_count" ] && [ "$transpose_mode" = allatonce ]; then
                sample_intervals=$SOLUTION_SAMPLE_INTERVALS
            fi
            echo "=== N=$grid_size P=$num_ranks $transpose_mode ==="
            mpirun -np "$num_ranks" --map-by core --bind-to core ./test_poisson \
                "$grid_size" "$grid_size" 1 1 "$transpose_mode" \
                "$NUM_REPS" "$sample_intervals" "$OUTPUT_DIR"
        done
    done
done

# One merged table of every timing row in the output directory.
awk 'FNR>1 || NR==1' "$OUTPUT_DIR"/timing_P*.csv > "$OUTPUT_DIR/timing.csv"
echo "merged timing rows into $OUTPUT_DIR/timing.csv"
