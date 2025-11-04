#!/usr/bin/env bash
# Usage: ./run_inception_stats.sh <num_runs>
# Example: ./run_inception_stats.sh 20

set -euo pipefail

# ----- configuration -----
ADDR="0xffffffffc03c63e0" #kmod address
RUNS=50
LOGFILE="inception_results.log"

# ----- run the command X times -----
echo "[*] Running ./inception ${ADDR} ${RUNS} times..."
: > "$LOGFILE"   # truncate log
for ((i=1; i<=RUNS; i++)); do
    echo "[*] Run #$i" | tee -a "$LOGFILE"
    ./inception0 "$ADDR" 2>&1 | tee -a "$LOGFILE" || \
        echo "[!] Run #$i failed" | tee -a "$LOGFILE"
    echo "" >> "$LOGFILE"
done

echo "[*] All runs complete. Results logged to $LOGFILE"

