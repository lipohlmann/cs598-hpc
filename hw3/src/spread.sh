#!/bin/bash
# Pin this MPI rank to one core, dealing ranks round-robin over the L3
# domains (CCDs: 8 cores sharing one L3 and one memory link), so rank r
# goes to CCD r % (number of CCDs).  Used by sweep.sbatch / run.sbatch as
#
#   mpirun --bind-to none -np N ./spread.sh ./prog args...
#
# Uses OS cpu numbers from /sys, so it does not depend on how hwloc
# numbers hardware threads (which differs between the cluster's nodes).
if [ -z "$SPREAD_CPUS" ]; then
    # allowed cpus, tagged with their L3 id, ordered (index in L3, L3 id)
    SPREAD_CPUS=$(awk '/Cpus_allowed_list/ {print $2}' /proc/self/status |
        tr ',' '\n' | awk -F- '{for (i = $1; i <= (NF > 1 ? $2 : $1); i++) print i}' |
        while read -r c; do
            echo "$(cat /sys/devices/system/cpu/cpu$c/cache/index3/id) $c"
        done | awk '{print n[$1]++, $1, $2}' | sort -k1,1n -k2,2n |
        awk '{printf "%s%s", sep, $3; sep = ","}')
fi
if [ $# -eq 0 ]; then              # no program: just print the cpu order
    echo "$SPREAD_CPUS"
    exit 0
fi
IFS=, read -r -a cpus <<< "$SPREAD_CPUS"
exec taskset -c "${cpus[${OMPI_COMM_WORLD_RANK:-0} % ${#cpus[@]}]}" "$@"
