#!/usr/bin/env python3
"""Parse RESULT lines from sweep logs, write results.csv, make the Q1 plots.

    python3 plot_results.py lu-sweep-*.log -o ../report/figs

Efficiency of a run = (its GFLOPS per rank) / (GFLOPS of the 1-rank run with
the same n, else the 1-rank run with the same n^2/(P Q)).
"""
import argparse
import glob
import math
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

COLS = ["m", "n", "nb", "P", "Q", "np", "time", "gflops", "gflops_rank", "err"]
PALETTE = ["#0072B2", "#D55E00", "#009E73", "#CC79A7", "#E69F00", "#56B4E9", "#000000", "#882255", "#999999"]
MARKERS = "osD^vPXhp"
ERR_TOL = 1e-8


def parse(paths):
    rows = []
    for p in paths:
        with open(p, "rb") as f:                     # OpenMPI may leave NUL bytes
            for raw in f:
                if b"RESULT" in raw:
                    t = re.sub(r"\x00", "", raw.decode(errors="ignore")).split()
                    i = t.index("RESULT")
                    rows.append([float(x) for x in t[i + 1:i + 1 + len(COLS)]])
    df = pd.DataFrame(rows, columns=COLS)
    df[COLS[:6]] = df[COLS[:6]].astype(int)
    return df


def add_derived(df):
    df = (df.groupby(["m", "n", "nb", "P", "Q", "np"], as_index=False)
            .agg(time=("time", "median"), gflops=("gflops", "median"),
                 gflops_rank=("gflops_rank", "median"), err=("err", "max")))
    df["foot"] = df["m"] ** 2 / df["np"]            # n^2/(P Q), doubles per rank
    df["foot_key"] = df["foot"].map(lambda x: float(f"{x:.1e}"))
    one = df[df["np"] == 1]
    by_n = one.set_index("m")["gflops_rank"]
    by_f = one.assign(k=one["foot"].map(lambda x: float(f"{x:.1e}"))).set_index("k")["gflops_rank"]
    base = [by_n.get(m, by_f.get(k, np.nan)) for m, k in zip(df["m"], df["foot_key"])]
    df["eff"] = df["gflops_rank"] / np.array(base)
    t1 = one.set_index("m")["time"]
    df["speedup"] = [t1.get(m, np.nan) / t for m, t in zip(df["m"], df["time"])]
    return df.sort_values(["m", "np"])


def style(ax, xlabel, ylabel, title):
    ax.set_xlabel(xlabel); ax.set_ylabel(ylabel); ax.set_title(title)
    ax.grid(True, which="both", alpha=0.25, lw=0.6)
    ax.spines[["top", "right"]].set_visible(False)


def series(ax, x, y, i, label):
    ax.plot(x, y, marker=MARKERS[i % len(MARKERS)], color=PALETTE[i % len(PALETTE)], lw=1.8, ms=6, label=label)


def strong_plot(df, out):
    groups = [(m, g) for m, g in df.groupby("m") if g["np"].nunique() >= 3]
    if not groups:
        return
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 4.2))
    for i, (m, g) in enumerate(groups):
        series(a1, g["np"], g["time"], i, f"n={m}")
        series(a2, g["np"], g["speedup"], i, f"n={m}")
        if g["np"].min() == 1:
            a1.plot(g["np"], g["time"].iloc[0] / g["np"], "--", color=PALETTE[i % len(PALETTE)], alpha=0.4, lw=1)
    npmax = df["np"].max()
    a2.plot([1, npmax], [1, npmax], "k--", lw=1, alpha=0.5, label="ideal")
    for a in (a1, a2):
        a.set_xscale("log", base=2); a.set_yscale("log", base=2 if a is a2 else 10)
    style(a1, "ranks (P Q)", "time (s)", "Strong scaling: time (dashed = ideal)")
    style(a2, "ranks (P Q)", "speedup vs 1 rank", "Strong scaling: speedup")
    a1.legend(); a2.legend()
    fig.tight_layout(); fig.savefig(f"{out}/strong.png", dpi=200); plt.close(fig)


def weak_plot(df, out):
    groups = [(k, g) for k, g in df.groupby("foot_key") if g["np"].nunique() >= 3]
    if not groups:
        return
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 4.2))
    for i, (k, g) in enumerate(groups):
        lab = f"n²/PQ = {k:.3g}"
        series(a1, g["np"], g["time"], i, lab)
        series(a2, g["np"], g["eff"], i, lab)
    a2.axhline(0.8, color="k", ls=":", lw=1); a2.set_ylim(0, 1.15)
    for a in (a1, a2):
        a.set_xscale("log", base=2)
    a1.set_yscale("log")
    style(a1, "ranks (P Q)", "time (s)", "Weak scaling: time (flops/rank grow as sqrt(PQ))")
    style(a2, "ranks (P Q)", "efficiency (GFLOPS/rank vs 1 rank)", "Weak scaling: efficiency (dotted = 80%)")
    a1.legend(); a2.legend()
    fig.tight_layout(); fig.savefig(f"{out}/weak.png", dpi=200); plt.close(fig)


def gflops_plot(df, out):
    groups = [(k, g) for k, g in df.groupby("foot_key") if g["np"].nunique() >= 3]
    if not groups:                                  # nothing to draw
        return
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 4.2))
    for i, (k, g) in enumerate(groups):
        series(a1, g["np"], g["gflops"], i, f"n²/PQ = {k:.3g}")
        series(a2, g["np"], g["gflops_rank"], i, f"n²/PQ = {k:.3g}")
    for a in (a1, a2):
        a.set_xscale("log", base=2)
    a1.set_yscale("log")
    style(a1, "ranks (P Q)", "total GFLOPS", "Total GFLOPS (weak-scaling runs)")
    style(a2, "ranks (P Q)", "GFLOPS per rank", "GFLOPS per rank")
    a2.set_ylim(bottom=0); a1.legend()
    fig.tight_layout(); fig.savefig(f"{out}/gflops.png", dpi=200); plt.close(fig)


def footprint_plot(df, out):
    d = df.dropna(subset=["eff"])
    fig, ax = plt.subplots(figsize=(6.5, 4.4))
    rows = []
    for i, (p, g) in enumerate(d[d["np"] > 1].groupby("np")):
        g = g.sort_values("foot")
        series(ax, g["foot"], g["eff"], i, f"{p} ranks")
        x, y = np.log(g["foot"].values), g["eff"].values
        ok = np.where(y >= 0.8)[0]
        if len(ok):
            j = ok[0]
            f = math.exp(x[j]) if j == 0 else math.exp(
                x[j - 1] + (0.8 - y[j - 1]) * (x[j] - x[j - 1]) / (y[j] - y[j - 1]))
        else:
            f = np.nan
        rows.append((p, f))
    ax.axhline(0.8, color="k", ls=":", lw=1)
    ax.set_xscale("log")
    style(ax, "local footprint n²/(P Q)  [doubles per rank]", "parallel efficiency", "Efficiency vs local footprint")
    ax.legend(ncol=2, fontsize=8)
    fig.tight_layout(); fig.savefig(f"{out}/efficiency_footprint.png", dpi=200); plt.close(fig)
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("logs", nargs="+")
    ap.add_argument("-o", "--out", default=".")
    ap.add_argument("--csv", default="results.csv")
    args = ap.parse_args()
    import os
    os.makedirs(args.out, exist_ok=True)

    paths = [q for p in args.logs for q in glob.glob(p)]
    df = parse(paths)
    if df.empty:
        raise SystemExit("no RESULT lines found")
    df = df[df["nb"] == 1]                      # Q1 scaling uses nb = 1 only
    bad = df[(df["err"] > ERR_TOL) | (df["err"] < 0) & (df["n"] == df["m"] + 1)]
    if len(bad):
        print(f"WARNING: {len(bad)} runs with rel_err > {ERR_TOL}:\n{bad}")
    df = add_derived(df)
    df.to_csv(args.csv, index=False)

    strong_plot(df, args.out); weak_plot(df, args.out); gflops_plot(df, args.out)
    rows = footprint_plot(df, args.out)

    pd.set_option("display.width", 160)
    print(df[["m", "nb", "P", "Q", "np", "time", "gflops", "gflops_rank", "eff", "err"]].to_string(index=False))
    print(f"\nPeak: {df.gflops.max():.2f} GFLOPS total, {df.gflops_rank.max():.2f} GFLOPS/rank")
    print("Smallest n²/(P Q) with >= 80% efficiency (interpolated):")
    for p, f in rows:
        print(f"  {p:4d} ranks: " + (f"{f:,.0f} doubles ({f * 8 / 2**20:.1f} MiB)" if f == f else "not reached"))


if __name__ == "__main__":
    main()
