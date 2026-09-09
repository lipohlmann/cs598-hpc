#!/bin/bash
# Short shared-memory smoke test for the ping-pong benchmark.
#
# Not a measurement run -- an oversubscribed laptop/WSL box is far too noisy for
# that, and every partner is on one node.  Real numbers come from
# batch/submit_all.sh on the campus cluster.  The full 500-size sweep is the
# default anyway: it costs about a second here, and a truncated sweep stops
# short of the eager-limit knee, which is most of what this test is for.

set -euo pipefail
cd "$(dirname "$0")"

NP=${NP:-8}
NSIZES=${NSIZES:-500}
MSG_VOL=${MSG_VOL:-1e5}
OUT=${OUT:-data/local_P${NP}.csv}

mkdir -p data

mpirun -np "$NP" --oversubscribe ./pingpong \
    --msg-vol "$MSG_VOL" \
    --nsizes "$NSIZES" \
    --nodes 1 \
    --verify \
    -o "$OUT"

echo
echo "rows: $(grep -vc '^#' "$OUT") (expect $(( NP * NSIZES )) + 1 header)"
