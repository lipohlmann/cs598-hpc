#!/usr/bin/env python3
"""Parse RESULT lines from sweep logs, write results.csv, make the Q1 plots.

    python3 plot_results.py lu-sweep-*.log -o ../report/figs

Weak-scaling efficiency of a run = (its GFLOPS per rank) / (GFLOPS of the 1-rank run
with the same n^2/(P Q)); footprint plots divide by the best 1-rank GFLOPS.
"""
import argparse
import glob
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
    rows, kinds = [], []
    for p in paths:
        kind = "strong,weak"                         # logs without a "cases:" line
        with open(p, "rb") as f:                     # OpenMPI may leave NUL bytes
            for raw in f:
                if raw.startswith(b"cases:"):        # which sweep the log is from
                    name = raw.decode(errors="ignore")
                    kind = next((k for k in ("strong", "weak") if k in name), "other")
                if b"RESULT" in raw:
                    t = re.sub(r"\x00", "", raw.decode(errors="ignore")).split()
                    i = t.index("RESULT")
                    rows.append([float(x) for x in t[i + 1:i + 1 + len(COLS)]])
                    kinds.append(kind)
    df = pd.DataFrame(rows, columns=COLS)
    df[COLS[:6]] = df[COLS[:6]].astype(int)
    df["kind"] = kinds
    return df


def add_derived(df):
    df = (df.groupby(["m", "n", "nb", "P", "Q", "np"], as_index=False)
            .agg(time=("time", "median"), gflops=("gflops", "median"),
                 gflops_rank=("gflops_rank", "median"), err=("err", "max"),
                 kind=("kind", lambda k: ",".join(sorted(set(",".join(k).split(",")))))))
    df["foot"] = df["m"] ** 2 / df["np"]            # n^2/(P Q), doubles per rank
    df["foot_key"] = df["foot"].map(lambda x: float(f"{x:.1e}"))
    one = df[df["np"] == 1]
    # weak-scaling efficiency: vs the 1-rank run with the same n^2/(P Q)
    by_f = one.assign(k=one["foot"].map(lambda x: float(f"{x:.1e}"))).set_index("k")["gflops_rank"]
    df["eff"] = df["gflops_rank"] / df["foot_key"].map(by_f)
    df["eff_peak"] = df["gflops_rank"] / one["gflops_rank"].max()   # vs best 1-rank rate
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
    df = df[df["kind"].str.contains("strong")]
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
    df = df[df["kind"].str.contains("weak")]
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
    df = df[df["kind"].str.contains("weak")]
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


def threshold_80(foot, eff):
    """Smallest measured footprint from which every larger measured footprint
    also reaches 80% efficiency; nan if the largest one does not."""
    below = np.where(eff < 0.8)[0]
    if not len(below):
        return foot[0]
    j = below[-1] + 1
    return foot[j] if j < len(foot) else np.nan


def footprint_plot(df, out):
    d = df.dropna(subset=["eff_peak"])
    fig, ax = plt.subplots(figsize=(6.5, 4.4))
    rows = []
    for i, (p, g) in enumerate(d[d["np"] > 1].groupby("np")):
        g = g.sort_values("foot")
        series(ax, g["foot"], g["eff_peak"], i, f"{p} ranks")
        j = g["eff_peak"].idxmax()
        rows.append((p, threshold_80(g["foot"].values, g["eff_peak"].values),
                     g.loc[j, "eff_peak"], g.loc[j, "foot"]))
    ax.axhline(0.8, color="k", ls=":", lw=1)
    ax.set_xscale("log"); ax.set_ylim(bottom=0)
    style(ax, "local footprint n²/(P Q)  [doubles per rank]",
          "efficiency (GFLOPS/rank vs best 1-rank)", "Efficiency vs local footprint")
    ax.legend(ncol=2, fontsize=8)
    fig.tight_layout(); fig.savefig(f"{out}/efficiency_footprint.png", dpi=200); plt.close(fig)
    return rows


L3_BYTES = 16 * 32 * 2**20     # 2 sockets x 8 CCDs x 32 MiB on the EPYC 7713 nodes


def total_plot(df, out):
    """Efficiency against the size of the whole matrix (8 n^2 bytes): the
    rank-1 update streams the trailing matrix every step, so speed depends
    on whether it fits in the node's combined L3."""
    d = df[df["np"] > 1].dropna(subset=["eff_peak"])
    fig, ax = plt.subplots(figsize=(6.5, 4.4))
    for i, (p, g) in enumerate(d.groupby("np")):
        g = g.assign(mib=8 * g["m"] ** 2 / 2**20).sort_values("mib")
        series(ax, g["mib"], g["eff_peak"], i, f"{p} ranks")
    ax.axvline(L3_BYTES / 2**20, color="k", ls="--", lw=1)
    ax.text(L3_BYTES / 2**20, 0.02, " node L3 (512 MiB)", fontsize=8, ha="left", va="bottom")
    ax.axhline(0.8, color="k", ls=":", lw=1)
    ax.set_xscale("log"); ax.set_ylim(bottom=0)
    style(ax, "whole matrix 8 n²  [MiB]", "efficiency (GFLOPS/rank vs best 1-rank)",
          "Efficiency vs total matrix size")
    ax.legend(ncol=2, fontsize=8)
    fig.tight_layout(); fig.savefig(f"{out}/efficiency_total.png", dpi=200); plt.close(fig)


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
    total_plot(df, args.out)

    pd.set_option("display.width", 160)
    print(df[["m", "nb", "P", "Q", "np", "time", "gflops", "gflops_rank", "eff", "eff_peak", "err"]].to_string(index=False))
    print(f"\nPeak: {df.gflops.max():.2f} GFLOPS total, {df.gflops_rank.max():.2f} GFLOPS/rank")
    print("Smallest measured n²/(P Q) from which every run reaches >= 80% of the best 1-rank GFLOPS:")
    for p, f, e, fe in rows:
        print(f"  {p:4d} ranks: " + (f"{f:,.0f} doubles ({f * 8 / 2**20:.1f} MiB)" if f == f else "not reached")
              + f"   [best {e:.2f} at {fe:,.0f}]")


if __name__ == "__main__":
    main()
