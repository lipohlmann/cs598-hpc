#!/bin/bash
# Compile one .c driver (plus every other src/*.c that has no main) and
# submit it to the queue.  Rank count and time limit come from the
# environment; everything after the .c file is passed to the program.
#
#   ./submit.sh src/main.c
#   NP=16 ./submit.sh src/main.c 65536
#   NP=256 NODES=2 TIME=00:30:00 ./submit.sh src/main.c
#
# The binary goes in build/<name>, the job log in <name>-<jobid>.log.

set -e

if [ $# -lt 1 ] || [[ "$1" != *.c ]]; then
    echo "usage: [NP=4] [NODES=1] [TIME=00:10:00] ./submit.sh file.c [args...]" >&2
    exit 1
fi
SRC=$1
shift
if [ ! -f "$SRC" ]; then
    echo "$SRC not found" >&2
    exit 1
fi

NP=${NP:-4}
NODES=${NODES:-1}
TIME=${TIME:-00:10:00}
NAME=$(basename "$SRC" .c)
SRC=$(realpath "$SRC")
cd "$(dirname "$0")"

type module >/dev/null 2>&1 || source /usr/share/lmod/lmod/init/bash
module load openmpi

# The other files in src/ are the library; skip any that bring their own main.
LIBS=()
for f in src/*.c; do
    [ "$(realpath "$f")" = "$SRC" ] && continue
    grep -qE '\bint[[:space:]]+main[[:space:]]*\(' "$f" && continue
    LIBS+=("$f")
done

mkdir -p build
echo "compiling $SRC ${LIBS[*]} -> build/$NAME"
mpicc -O2 -Wall -Wextra -std=c11 -Iinclude "$SRC" "${LIBS[@]}" -o "build/$NAME" -lm

sbatch <<EOF
#!/bin/bash
#SBATCH --account=26fa-cs598pa-eng
#SBATCH --partition=eng-instruction
#SBATCH --time=$TIME
#SBATCH --nodes=$NODES
#SBATCH --ntasks=$NP
#SBATCH --cpus-per-task=1
#SBATCH --job-name=$NAME
#SBATCH --output=$NAME-%j.log

module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the log
export OMP_NUM_THREADS=1

echo "node \$(hostname), SLURM_NTASKS=\$SLURM_NTASKS"
echo "cmd: build/$NAME $*"
mpirun -np \$SLURM_NTASKS --map-by core --bind-to core build/$NAME $* </dev/null
EOF
