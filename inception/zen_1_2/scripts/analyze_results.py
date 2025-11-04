#!/usr/bin/env python3
import re
import sys
import statistics

EXPECTED_X_PER_RUN = 4096

# ChatGPT code generated to parse through the results file
# need to parse the results for 
# - Time to break KASLR
# - How many bytes/s 
# - How many X's were there (should be 4096 per run)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_results.py <logfile>")
        sys.exit(1)

    logfile = sys.argv[1]

    # Regex patterns
    kaslr_break_pattern = re.compile(r"Took\s+(\d+)\s+seconds to break KASLR", re.IGNORECASE)
    bytes_pattern = re.compile(r"(\d+)\s+bytes/s")

    run_info = []  # list of dicts: {"kaslr_time": int, "x_count": int, "x_acc": float, "bytes_per_s": int}

    in_run = False
    current_run_Xs = 0
    current_kaslr_time = None

    with open(logfile, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            # Capture KASLR time
            kaslr_match = kaslr_break_pattern.search(line)
            if kaslr_match:
                current_kaslr_time = int(kaslr_match.group(1))
                in_run = True
                current_run_Xs = 0
                continue

            # End of run: line containing "bytes in"
            bytes_match = bytes_pattern.search(line)
            if in_run and bytes_match:
                bytes_per_s = int(bytes_match.group(1))
                x_acc = (current_run_Xs / EXPECTED_X_PER_RUN) * 100
                run_info.append({
                    "kaslr_time": current_kaslr_time,
                    "x_count": current_run_Xs,
                    "x_acc": x_acc,
                    "bytes_per_s": bytes_per_s
                })
                # Reset run tracking
                in_run = False
                current_run_Xs = 0
                current_kaslr_time = None
                continue

            # Count Xs if inside a run
            if in_run:
                current_run_Xs += line.count("X")

    # Print per-run information
    print(f"{'Run#':>4} | {'KASLR(s)':>9} | {'X Count':>8} | {'X Accuracy(%)':>14} | {'Bytes/s':>8}")
    print("-" * 60)
    for i, info in enumerate(run_info, 1):
        print(f"{i:>4} | {info['kaslr_time']:>9} | {info['x_count']:>8} | {info['x_acc']:>14.2f} | {info['bytes_per_s']:>8}")

    # Summary statistics
    if run_info:
        bytes_list = [r['bytes_per_s'] for r in run_info]
        x_acc_list = [r['x_acc'] for r in run_info]
        kaslr_times = [r['kaslr_time'] for r in run_info]

        mean_bytes = statistics.mean(bytes_list)
        std_bytes = statistics.pstdev(bytes_list)
        avg_x_acc = statistics.mean(x_acc_list)
        avg_kaslr_time = statistics.mean(kaslr_times)

        print("\nSummary:")
        print(f"Mean bytes/s: {mean_bytes:.2f}, Std Dev bytes/s: {std_bytes:.2f}")
        print(f"Average X accuracy: {avg_x_acc:.2f}%")
        print(f"Average KASLR break time: {avg_kaslr_time:.2f} seconds")
    else:
        print("No runs found in the log!")

if __name__ == "__main__":
    main()

