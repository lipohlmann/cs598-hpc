#!/usr/bin/env python3
"""Weak-scaling plots from weak.csv (fixed N/P, varying P).

    python3 plot_weak.py [weak.csv] [-o OUTDIR]

Writes weak_time.png, weak_efficiency.png, weak_gflops.png.
Efficiency is t_mean(P=1) / t_mean(P).
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
    ap.add_argument("csv", nargs="?", default=os.path.join(HERE, "weak.csv"))
    ap.add_argument("-o", "--outdir", default=HERE)
    args = ap.parse_args()

    df = pd.read_csv(args.csv).sort_values("P")
    P, t, gf = df["P"].to_numpy(), df["t_mean"].to_numpy(), df["gflops"].to_numpy()
    yerr = [t - df["t_min"].to_numpy(), df["t_max"].to_numpy() - t]
    t1 = t[P == 1][0] if (P == 1).any() else t[0]
    gf1 = gf[P == 1][0] if (P == 1).any() else gf[0] / P[0]
    eff = t1 / t
    npr = int(df["N"].iloc[0] // df["P"].iloc[0])
    title = f"Weak scaling, N/P = {npr:,}"

    # time vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.errorbar(P, t * 1e3, yerr=[e * 1e3 for e in yerr], fmt="o-", color=COLOR, capsize=3,
                label="measured (mean, min/max)")
    ax.axhline(t1 * 1e3, ls="--", color=IDEAL, label="ideal (constant)")
    ax.set_xscale("log", base=2)
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_ylim(0, None)
    ax.set_xlabel("ranks P"); ax.set_ylabel("time per solve [ms]")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "weak_time.png"), dpi=150)

    # efficiency vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.plot(P, eff, "o-", color=COLOR, label="measured")
    ax.axhline(1.0, ls="--", color=IDEAL, label="ideal")
    ax.set_xscale("log", base=2)
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_ylim(0, max(1.1, eff.max() * 1.1))
    ax.set_xlabel("ranks P"); ax.set_ylabel("efficiency $t_1 / t_P$")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "weak_efficiency.png"), dpi=150)

    # aggregate GFLOP/s vs P
    fig, ax = plt.subplots(figsize=(6, 4.5))
    ax.plot(P, gf, "o-", color=COLOR, label="measured")
    ax.plot(P, gf1 * P, "--", color=IDEAL, label="ideal $P \\cdot$ GF$_1$")
    ax.set_xscale("log", base=2); ax.set_yscale("log")
    ax.set_xticks(P); ax.set_xticklabels(P)
    ax.set_xlabel("ranks P"); ax.set_ylabel("GFLOP/s")
    ax.set_title(title); ax.grid(True, which="both", alpha=0.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(args.outdir, "weak_gflops.png"), dpi=150)

    print(df.assign(efficiency=eff)[["P", "N", "t_mean", "efficiency", "gflops"]].to_string(index=False))


if __name__ == "__main__":
    main()
