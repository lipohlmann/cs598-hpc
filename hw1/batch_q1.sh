#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng     # This is valid for all students
#SBATCH --partition=eng-instruction    # <- replace with your partition
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --job-name=run_hw1
#SBATCH --output=optimized_logfile

echo "=== compute node CPU ==="
lscpu | grep -Ei 'model name|^cpu\(s\)|mhz|cache'
echo "=== vector ISA available ==="
lscpu | grep -Eo 'avx512f|avx512vl|avx2|\bfma\b' | sort -u
echo "=== ISA gcc -march=native actually resolves to on this node ==="
gcc -march=native -Q --help=target 2>/dev/null | grep -E '\-m(avx|fma|sse|arch=| tune=)'

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

make clean || true
make code
make run
python plot_gflops.py gflops.csv -o optimized.png
mv gflops.csv ./q1_results/
mv optimized.png ./q1_results/
mv optimized_logfile ./q1_results/

