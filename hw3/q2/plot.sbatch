#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=00:10:00
#SBATCH --job-name=lu-plot
#SBATCH --output=lu-plot-%j.log

# Run plot_results.py on the given sweep logs (submitted by run_all.sh).
#
#   sbatch plot.sbatch lu-sweep-123.log lu-sweep-124.log ...
#   PYTHON_MODULE=anaconda3 sbatch --export=ALL plot.sbatch ...   # force a module
#
# Needs python3 with numpy, pandas and matplotlib.  Loads the first module
# from the list below that provides all three.

cd "${SLURM_SUBMIT_DIR:-.}"

load_python() {
    local m
    for m in $PYTHON_MODULE anaconda3 anaconda miniconda3 python; do
        module load "$m" 2>/dev/null || continue
        if python3 -c "import numpy, pandas, matplotlib" 2>/dev/null; then
            echo "using python module $m"
            return 0
        fi
        module unload "$m" 2>/dev/null
    done
    return 1
}

if ! load_python; then
    echo "no python module with numpy, pandas and matplotlib found" >&2
    echo "set PYTHON_MODULE (see 'module avail'), or run plot_results.py elsewhere" >&2
    exit 1
fi

python3 plot_results.py "$@" -o ../report/figs
