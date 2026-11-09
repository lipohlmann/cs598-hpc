#!/bin/bash
# Compile one .cc driver (plus every other src/*.cc that has no main) and
# run it right here, e.g. on the login node for a quick correctness check.
# Keep NP small: the login node is shared, and timings from it mean nothing.
#
#   ./run_local.sh src/main.cc
#   NP=8 ./run_local.sh src/main.cc 65536
#
# The binary goes in build/<name>, same as submit.sh.

set -e

if [ $# -lt 1 ] || [[ "$1" != *.cc ]]; then
    echo "usage: [NP=4] ./run_local.sh file.cc [args...]" >&2
    exit 1
fi
SRC=$1
shift
if [ ! -f "$SRC" ]; then
    echo "$SRC not found" >&2
    exit 1
fi

NP=${NP:-4}
NAME=$(basename "$SRC" .cc)
SRC=$(realpath "$SRC")
cd "$(dirname "$0")"

type module >/dev/null 2>&1 || source /usr/share/lmod/lmod/init/bash
module load openmpi
export OMPI_MCA_ras_base_verbose=0     # module sets 10, which floods the output

# The other files in src/ are the library; skip any that bring their own main.
LIBS=()
for f in src/*.cc; do
    [ "$(realpath "$f")" = "$SRC" ] && continue
    grep -qE '\bint[[:space:]]+main[[:space:]]*\(' "$f" && continue
    LIBS+=("$f")
done

mkdir -p build
echo "compiling $SRC ${LIBS[*]} -> build/$NAME"
mpicxx -O2 -Wall -Wextra -std=c++17 -Iinclude "$SRC" "${LIBS[@]}" -o "build/$NAME" -lm

# The module defaults to UCX over InfiniBand, which login nodes don't have;
# use Open MPI's own shared-memory transport instead.
mpirun -np "$NP" --mca pml ob1 --mca btl self,vader "build/$NAME" "$@"
