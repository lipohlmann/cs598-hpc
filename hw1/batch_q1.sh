#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng     # This is valid for all students
#SBATCH --partition=eng-instruction    # <- replace with your partition
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8              # matches the 8-CPU MatLab comparison run
#SBATCH --mem=16G
#SBATCH --job-name=run_hw1
#SBATCH --output=logfile

module load miniconda3/24.9.2

# `module load` only puts a bare base Python on PATH (no matplotlib), and
# `conda activate` needs conda.sh sourced before it works in a non-login
# shell. Create (once) and activate a dedicated env with the plotting deps,
# falling back to a user pip install if conda can't provision one.
source "$(conda info --base)/etc/profile.d/conda.sh"
CONDA_ENV=hw1-plot
if ! conda env list | grep -qE "^${CONDA_ENV}[[:space:]]"; then
  conda create -y -n "${CONDA_ENV}" python=3.11 matplotlib || true
fi
if conda env list | grep -qE "^${CONDA_ENV}[[:space:]]"; then
  conda activate "${CONDA_ENV}"
fi
python -c "import matplotlib" 2>/dev/null || python -m pip install --user --quiet matplotlib

# Give the OpenMP kernel exactly the cores Slurm allocated to this task, and
# pin threads to distinct cores for stable, reproducible timings.
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK:-8}
export OMP_PROC_BIND=close
export OMP_PLACES=cores

make clean || true
make code
make run
python plot_gflops.py gflops.csv -o first_run.png
mv gflops.csv ./q1_results/
mv first_run.png ./q1_results/
mv logfile ./q1_results/

