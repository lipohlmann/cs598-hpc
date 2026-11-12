#!/usr/bin/env bash
# Shared settings and helpers for the study scripts. Source it, don't run it.
#
# Every setting can be overridden from the environment, e.g.
#   ACCOUNT=myacct PARTITION=cpu TASKS_PER_NODE=128 ./scripts/run_strong_scaling.sh
#
# Settings
#   EXE             path to the midterm executable      (default: ./midterm)
#   RESULTS_DIR     where CSVs, logs and VTK files go   (default: ./results)
#   SUBMIT          sbatch | local                      (default: sbatch)
#                     sbatch: one Slurm job per run, launched with $LAUNCHER
#                     local:  run immediately with $MPIRUN (laptop smoke tests)
#   DRY_RUN         1 = print the commands, run/submit nothing (default: 0)
#   ACCOUNT         Slurm account (--account), empty = not passed
#   PARTITION       Slurm partition (--partition), empty = not passed
#   TASKS_PER_NODE  MPI ranks per node                  (default: 64)
#   TIME_LIMIT      Slurm time limit per run            (default: 00:30:00)
#   EXCLUSIVE       1 = request whole nodes (--exclusive, recommended for
#                   timing studies)                     (default: 1)
#   SBATCH_EXTRA    extra sbatch arguments, e.g. "--mem=0 --constraint=xyz"
#   SETUP_CMD       shell commands run in the job before the launch, e.g.
#                   "module load openmpi/4.1"           (default: empty)
#   LAUNCHER        MPI launcher inside a job           (default: srun --mpi=pmix --cpu-bind=cores)
#   MPIRUN          MPI launcher for SUBMIT=local       (default: mpirun --oversubscribe)
#   REPS            repetitions of each timing run      (default: 5)

set -euo pipefail

EXE=${EXE:-./midterm}
RESULTS_DIR=${RESULTS_DIR:-./results}
SUBMIT=${SUBMIT:-sbatch}
DRY_RUN=${DRY_RUN:-0}
ACCOUNT=${ACCOUNT:-26fa-cs598pa-eng}
PARTITION=${PARTITION:-eng-instruction}
TASKS_PER_NODE=${TASKS_PER_NODE:-64}
TIME_LIMIT=${TIME_LIMIT:-00:30:00}
EXCLUSIVE=${EXCLUSIVE:-1}
SBATCH_EXTRA=${SBATCH_EXTRA:-}
SETUP_CMD=${SETUP_CMD:-}
LAUNCHER=${LAUNCHER:-srun --mpi=pmix --cpu-bind=cores}
MPIRUN=${MPIRUN:-mpirun --oversubscribe}
REPS=${REPS:-5}

if [[ "$SUBMIT" != "sbatch" && "$SUBMIT" != "local" ]]; then
  echo "error: SUBMIT must be 'sbatch' or 'local' (got '$SUBMIT')" >&2
  exit 1
fi
if [[ "$DRY_RUN" != "1" && ! -x "$EXE" ]]; then
  echo "error: executable '$EXE' not found; run 'make code' first or set EXE" >&2
  exit 1
fi

# submit_run STUDY LABEL RANKS [midterm arguments...]
#
# Runs (SUBMIT=local) or submits (SUBMIT=sbatch) one execution of the program
# with RANKS MPI ranks. The results go to $RESULTS_DIR/STUDY/LABEL.csv and the
# program output to $RESULTS_DIR/logs/STUDY-LABEL[-jobid].out.
submit_run() {
  local study=$1 label=$2 ranks=$3
  shift 3

  local csv="$RESULTS_DIR/$study/$label.csv"
  local log_base="$RESULTS_DIR/logs/$study-$label"
  local args=("$@" --csv "$csv" --label "$label")

  if [[ "$DRY_RUN" != "1" ]]; then
    mkdir -p "$RESULTS_DIR/$study" "$RESULTS_DIR/logs"
  fi

  if [[ "$SUBMIT" == "local" ]]; then
    # shellcheck disable=SC2206  # MPIRUN is intentionally word-split
    local cmd=($MPIRUN -np "$ranks" "$EXE" "${args[@]}")
    if [[ "$DRY_RUN" == "1" ]]; then
      printf '%q ' "${cmd[@]}"
      echo
    else
      echo ">> $study/$label ($ranks ranks)"
      "${cmd[@]}" >"$log_base.out" 2>&1 || {
        echo "   FAILED, see $log_base.out" >&2
        return 1
      }
      grep -E "^ranks " "$log_base.out" | sed 's/^/   /'
    fi
    return 0
  fi

  # Slurm: one job per run
  local nodes=$(((ranks + TASKS_PER_NODE - 1) / TASKS_PER_NODE))
  local per_node=$((ranks < TASKS_PER_NODE ? ranks : TASKS_PER_NODE))

  local run_cmd
  run_cmd="$LAUNCHER $(printf '%q ' "$EXE" "${args[@]}")"
  if [[ -n "$SETUP_CMD" ]]; then
    run_cmd="$SETUP_CMD && $run_cmd"
  fi

  local sbatch_args=(
    --job-name "$study-$label"
    --nodes "$nodes"
    --ntasks "$ranks"
    --ntasks-per-node "$per_node"
    --time "$TIME_LIMIT"
    --output "$log_base-%j.out"
  )
  [[ -n "$ACCOUNT" ]] && sbatch_args+=(--account "$ACCOUNT")
  [[ -n "$PARTITION" ]] && sbatch_args+=(--partition "$PARTITION")
  [[ "$EXCLUSIVE" == "1" ]] && sbatch_args+=(--exclusive)
  if [[ -n "$SBATCH_EXTRA" ]]; then
    # shellcheck disable=SC2206  # SBATCH_EXTRA is intentionally word-split
    sbatch_args+=($SBATCH_EXTRA)
  fi

  if [[ "$DRY_RUN" == "1" ]]; then
    printf 'sbatch '
    printf '%q ' "${sbatch_args[@]}"
    printf -- '--wrap %q\n' "$run_cmd"
  else
    local job_id
    job_id=$(sbatch --parsable "${sbatch_args[@]}" --wrap "$run_cmd")
    echo "submitted $study/$label ($ranks ranks, $nodes node(s)): job $job_id"
  fi
}

# powers_of_two_up_to MAX -> "1 2 4 ... <= MAX"
powers_of_two_up_to() {
  local p=1 out=""
  while ((p <= $1)); do
    out+="$p "
    p=$((p * 2))
  done
  echo "$out"
}
