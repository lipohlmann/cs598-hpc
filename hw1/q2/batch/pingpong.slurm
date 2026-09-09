#!/bin/bash
# Generic ping-pong job.  Node and rank counts come from the sbatch command
# line, so this one file covers every configuration in Q2:
#
#   sbatch --nodes=1 --ntasks-per-node=64  batch/pingpong.slurm
#
# Submit from the hw1/q2 directory (batch/submit_all.sh does all five).
#
#SBATCH --account=26fa-cs598pa-eng     # valid for all students in the course
#SBATCH --partition=eng-instruction
#SBATCH --time=00:10:00                # keep under 10 minutes (see scripts/README.md)
#SBATCH --cpus-per-task=1
#SBATCH --job-name=pingpong
#SBATCH --output=logs/%x-P%N-%j.out

set -euo pipefail
cd "${SLURM_SUBMIT_DIR:-.}"
mkdir -p data logs

module load openmpi

NTASKS=${SLURM_NTASKS:-$(( SLURM_NNODES * SLURM_NTASKS_PER_NODE ))}
OUT=data/pp_P${NTASKS}_n${SLURM_NNODES}.csv

# MSG_VOL sets nloop = msg_vol/(nwds+2), clamped to [20,1000].  1e5 keeps a
# full 256-rank sweep to a few minutes.  Drop NSIZES below 500 (which truncates
# the largest messages) if a run ever threatens the time limit -- msg_vol is
# the weaker knob, because the nloop>=20 floor dominates at large m.
MSG_VOL=${MSG_VOL:-1e5}
NSIZES=${NSIZES:-500}

# Only ranks 0 and p are measured; the rest sit in MPI_Barrier, which Open MPI
# polls aggressively.  --bind-to core keeps those pollers off the two cores
# under test.  If the intra-node curves still look noisy, re-run with the line
# below uncommented and compare -- it makes idle ranks yield instead of spin.
# export OMPI_MCA_mpi_yield_when_idle=1

make --no-print-directory

echo "=== $(date) P=$NTASKS nodes=$SLURM_NNODES msg_vol=$MSG_VOL ==="
srun hostname -s | sort | uniq -c

mpirun -np "$NTASKS" \
    --map-by "ppr:${SLURM_NTASKS_PER_NODE}:node" \
    --bind-to core \
    ./pingpong \
        --msg-vol "$MSG_VOL" \
        --nsizes "$NSIZES" \
        --nodes "$SLURM_NNODES" \
        -o "$OUT"

echo "=== wrote $OUT ==="
python3 analysis/pingpong_stats.py "$OUT" || true
