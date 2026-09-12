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
# eng-instruction is heterogeneous: ccc0391-93 have 128 cores, ccc0398-99 have
# 64.  Left alone, a 2-node job can draw one of each, so Open MPI resolves
# fewer slots than 2 x ntasks-per-node and refuses to launch; worse, the 1-node
# 64- and 128-rank runs land on different CPUs and are not comparable.  Every
# config here fits on a 128-core node, so keep them all on that class by name.
# (--mincpus=128 is the tidier way to say this, but Slurm rejects it here with
# "Requested node configuration is not available" even for the 64-rank job.)
# Recheck the node list against `sinfo -p eng-instruction -o "%n %c"` if a job
# ever lands somewhere unexpected -- this does not self-update.
#SBATCH --exclude=ccc0398,ccc0399
#SBATCH --job-name=pingpong
# logs/ must already exist AT SUBMIT TIME -- Slurm opens this file before the
# script below runs, so the mkdir cannot cover it.  If logs/ is missing the job
# dies at launch with no output anywhere, which is confusing to chase.  Submit
# through batch/submit_all.sh, which creates it; a bare sbatch needs a manual
# `mkdir -p logs` first.
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

# srun, not mpirun: mpirun here does not read the Slurm allocation.  On a
# 2-node job it sees only the local node, so it either maps half the ranks
# ("Procs mapped: 64 ... PPR: 64:node") or piles all of them onto node 1
# ("binding more processes than cpus available in your allocation") depending
# on --map-by.  No mapping flag can fix that -- the node list is already
# missing.  The `srun hostname` line above proves srun sees both nodes, so let
# Slurm launch the ranks and drop -np/--map-by/--bind-to with it: SLURM_NTASKS
# sets the count, block distribution keeps ranks 0..n-1 on the first node, and
# --cpu-bind=cores replaces --bind-to core.
#
# If srun rejects the PMI type, `srun --mpi=list` prints what this build has
# (pmi2 is the usual alternative) -- override without editing via SRUN_MPI.
SRUN_MPI=${SRUN_MPI:-pmix}

srun --mpi="$SRUN_MPI" \
    --cpu-bind=cores \
    ./pingpong \
        --msg-vol "$MSG_VOL" \
        --nsizes "$NSIZES" \
        --nodes "$SLURM_NNODES" \
        -o "$OUT"

echo "=== wrote $OUT ==="
python3 analysis/pingpong_stats.py "$OUT" || true
