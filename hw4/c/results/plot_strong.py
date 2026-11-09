#!/usr/bin/env python3
"""Strong-scaling plots from strong.csv (fixed N, varying P).

    python3 plot_strong.py [strong.csv] [-o OUTDIR]

Writes strong_time.png, strong_speedup.png, strong_efficiency.png.
Speedup/efficiency are computed from t_mean relative to the P=1 run.
"""
import argparse
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

HERE = os.path.dirname(os.path.abspath(__file__))
COLOR, IDEAL = "#0072B2", "#999999"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="?", default=os.path.join(HERE, "strong.csv"))
    ap.add_argument("-o", "--outdir", default=HERE)
    args = ap.parse_args()

    df = pd.read_csv(args.csv).sort_values("P")
    P, t = df["P"].to_numpy(), df["t_mean"].to_numpy()
    yerr = [t - df["t_min"].to_numpy(), df["t_max"].to_numpy() - t]
    t1 = t[P == 1][0] if (P == 1).any() else t[0] * P[0]
    speedup = t1 / t
    eff = speedup / P
    N = df["N"].iloc[0]
    title = f"Strong scaling, N = {N:,}"

    # time vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.errorbar(P, t, yerr=yerr, fmt="o-", color=COLOR, capsize=3, label="measured (mean, min/max)")
    ax.plot(P, t1 / P, "--", color=IDEAL, label="ideal $t_1/P$")
    ax.set_xscale("log", base=2); ax.set_yscale("log")
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_xlabel("ranks P"); ax.set_ylabel("time per solve [s]")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "strong_time.png"), dpi=150)

    # speedup vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.plot(P, speedup, "o-", color=COLOR, label="measured")
    ax.plot(P, P, "--", color=IDEAL, label="ideal")
    ax.set_xscale("log", base=2); ax.set_yscale("log", base=2)
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_xlabel("ranks P"); ax.set_ylabel("speedup $t_1 / t_P$")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "strong_speedup.png"), dpi=150)

    # parallel efficiency vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.plot(P, eff, "o-", color=COLOR, label="measured")
    ax.axhline(1.0, ls="--", color=IDEAL, label="ideal")
    ax.set_xscale("log", base=2)
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_ylim(0, max(1.1, eff.max() * 1.1))
    ax.set_xlabel("ranks P"); ax.set_ylabel("efficiency $t_1 / (P\\,t_P)$")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "strong_efficiency.png"), dpi=150)

    print(df.assign(speedup=speedup, efficiency=eff)[["P", "t_mean", "speedup", "efficiency", "gflops"]]
          .to_string(index=False))


if __name__ == "__main__":
    main()
