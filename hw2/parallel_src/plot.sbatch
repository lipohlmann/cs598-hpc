#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --job-name=plot
#SBATCH --output=plot-%j.log

# Make every figure from the CSV files in the output directory.
#
#   sbatch plot.sbatch                              # results -> ../report/figures
#   sbatch plot.sbatch my_results my_figures
#   PYTHON_MODULE=anaconda3 sbatch plot.sbatch      # if "python" is not the module
#
# Needs a Python with numpy, pandas and matplotlib.

OUTPUT_DIR=${1:-results}
FIGURES_DIR=${2:-../report/figures}
PYTHON_MODULE=${PYTHON_MODULE:-python}

module load "$PYTHON_MODULE"

if ! python3 -c "import numpy, pandas, matplotlib" 2>/dev/null; then
    echo "python3 from module '$PYTHON_MODULE' lacks numpy, pandas or matplotlib;" >&2
    echo "set PYTHON_MODULE, or run plot_results.py where they are installed" >&2
    exit 1
fi

python3 plot_results.py "$OUTPUT_DIR" "$FIGURES_DIR"
