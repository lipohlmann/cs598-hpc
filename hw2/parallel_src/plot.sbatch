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
#   PYTHON_MODULE=anaconda3/2024.10 sbatch plot.sbatch   # name a module yourself
#
# Needs a Python with numpy, pandas and matplotlib.  The modules below
# are tried in turn until one provides them.

OUTPUT_DIR=${1:-results}
FIGURES_DIR=${2:-../report/figures}
CANDIDATE_PYTHON_MODULES="$PYTHON_MODULE anaconda3 anaconda miniconda3 python"

found_python=no
for python_module in $CANDIDATE_PYTHON_MODULES; do
    module load "$python_module" 2>/dev/null
    if python3 -c "import numpy, pandas, matplotlib" 2>/dev/null; then
        echo "using python module $python_module"
        found_python=yes
        break
    fi
    module unload "$python_module" 2>/dev/null
done
if [ "$found_python" = no ]; then
    echo "none of these modules has numpy, pandas and matplotlib: $CANDIDATE_PYTHON_MODULES" >&2
    echo "set PYTHON_MODULE (see 'module avail'), or run plot_results.py locally" >&2
    exit 1
fi

python3 plot_results.py "$OUTPUT_DIR" "$FIGURES_DIR"
