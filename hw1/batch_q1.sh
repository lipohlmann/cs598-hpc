#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng     # This is valid for all students
#SBATCH --partition=eng-instruction    # <- replace with your partition
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --job-name=run_octave
#SBATCH --output=logfile

module load miniconda3/24.9.2

make clean
make code 
make run
python plot_gflops.py gflops.csv -o first_run.png
mv gflops.csv ./q1_results/
mv first_run.png ./q1_results/
mv logfile ./q1_results/

