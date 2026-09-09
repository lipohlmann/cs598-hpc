#!/usr/bin/env python3
"""Plot GFLOPS/core vs. matrix size from the C benchmark's CSV output.

Mirrors the plot produced by hw1_matl/dgtime.m:
    plot(NN,gc,'ro-',lw,2);
    xlabel('Matrix Size, N',fs,20);
    ylabel('GFLOPS',fs,20);
    title ('Matlab C=AB GFLOPS per Core',fs,20);
    exportgraphics(gcf, 'octave_gflops.png', 'Resolution', 300)
"""

import argparse
import csv

import matplotlib.pyplot as plt


def read_csv(csv_path):
    Ns = []
    gcs = []
    with open(csv_path, newline="") as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        for row in reader:
            Ns.append(float(row[0]))
            gcs.append(float(row[1]))
    return Ns, gcs


def main():
    parser = argparse.ArgumentParser(
        description="Plot GFLOPS/core vs. matrix size from a CSV file."
    )
    parser.add_argument(
        "csv_path", help="Path to CSV file with N,GFLOPS_per_core columns"
    )
    parser.add_argument(
        "-o",
        "--output",
        default="c_gflops.png",
        help="Output image path (default: c_gflops.png)",
    )
    args = parser.parse_args()

    Ns, gcs = read_csv(args.csv_path)

    plt.plot(Ns, gcs, "ro-", linewidth=2)
    plt.xlabel("Matrix Size, N", fontsize=20)
    plt.ylabel("GFLOPS", fontsize=20)
    plt.title("C C=AB GFLOPS per Core", fontsize=20)
    plt.tight_layout()
    plt.savefig(args.output, dpi=300)


if __name__ == "__main__":
    main()
