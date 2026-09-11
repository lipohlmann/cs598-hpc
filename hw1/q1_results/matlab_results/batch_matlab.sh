#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng     # This is valid for all students
#SBATCH --partition=eng-instruction    # <- replace with your partition
#SBATCH --time=01:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --job-name=run_matlab
#SBATCH --output=logfile

module load matlab/R2025b

matlab -batch "dgtime"

