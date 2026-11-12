#!/usr/bin/env python3
"""Post-process the study results written by the scripts in this directory.

Usage:
    python scripts/analyze.py [RESULTS_DIR]      (default: ./results)

Reads RESULTS_DIR/<study>/*.csv (one row per run, written by `midterm --csv`)
and writes figures to RESULTS_DIR/figures and tables (Markdown and CSV) to
RESULTS_DIR/tables. Studies that have no results are skipped.

Requires pandas and matplotlib.

Definitions
-----------
* Times are the maximum over ranks (the slowest rank sets the wall time).
  Repeated runs are combined with the median.
* Strong scaling, baseline = smallest rank count P0 with time T0:
      speedup S(P) = P0 * T0 / T(P),   efficiency E(P) = S(P) / P.
* Weak scaling: E(P) = T0 / T(P), computed for the total solve time and for
  the time per CG iteration (the unpreconditioned CG iteration count grows
  with the problem size, so the per-iteration efficiency isolates the parallel
  overhead).
* Time breakdown of the solve: Kronecker products, gather-scatter (gslib),
  Allreduce, the rest of the matvec (mask, loop overhead) and the rest of CG
  (vector updates, local dot products). The "rest" columns are differences of
  maxima over ranks, clipped at zero.
* Load imbalance = max / average over ranks of the Kronecker time.
"""

import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
import pandas as pd  # noqa: E402


def load(results: Path, study: str) -> pd.DataFrame:
    files = sorted((results / study).glob("*.csv"))
    if not files:
        return pd.DataFrame()
    df = pd.concat([pd.read_csv(f) for f in files], ignore_index=True)
    # iterations actually performed (iters is -1 when the cap was reached);
    # every CG iteration does exactly one matvec, plus one for the residual
    df["iters_done"] = df["n_matvec"] - 1
    return df


def write_table(df: pd.DataFrame, tables: Path, name: str, floatfmt: str = "{:.4g}"):
    df.to_csv(tables / f"{name}.csv", index=False)

    def fmt(v):
        if isinstance(v, (float, np.floating)):
            return floatfmt.format(v)
        return str(v)

    lines = ["| " + " | ".join(df.columns) + " |",
             "|" + "|".join("---" for _ in df.columns) + "|"]
    for _, row in df.iterrows():
        lines.append("| " + " | ".join(fmt(v) for v in row) + " |")
    (tables / f"{name}.md").write_text("\n".join(lines) + "\n")
    print(f"\n## {name}\n" + "\n".join(lines))


def p_convergence(df, figures, tables):
    df = df.sort_values(["nx", "npts"])
    fig, ax = plt.subplots(figsize=(6, 4.5))
    for nx, g in df.groupby("nx"):
        ax.semilogy(g["npts"], g["l2_error"], "o-", label=f"L2, {nx}^3 elements")
        ax.semilogy(g["npts"], g["linf_error"], "s--", label=f"Linf, {nx}^3 elements")
    ax.set_xlabel("GLL points per direction per element")
    ax.set_ylabel("error")
    ax.set_title("p-convergence")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(figures / "p_convergence.png", dpi=150)
    plt.close(fig)

    write_table(df[["nx", "npts", "dofs", "iters_done", "l2_error", "linf_error"]]
                .rename(columns={"iters_done": "cg_iters"}), tables, "p_convergence")


def h_convergence(df, figures, tables):
    df = df.copy()
    df["h"] = df["lx"] / df["nx"]
    df = df.sort_values(["npts", "h"], ascending=[True, False])
    rows = []
    fig, ax = plt.subplots(figsize=(6, 4.5))
    for npts, g in df.groupby("npts"):
        g = g.reset_index(drop=True)
        rate = [np.nan] + [
            np.log(g.l2_error[i - 1] / g.l2_error[i]) / np.log(g.h[i - 1] / g.h[i])
            for i in range(1, len(g))
        ]
        g["l2_rate"] = rate
        rows.append(g)
        line, = ax.loglog(g["h"], g["l2_error"], "o-", label=f"npts={npts} (p={npts - 1})")
        # reference slope p+1 through the finest point
        p1 = npts
        h_ref = np.array([g.h.iloc[0], g.h.iloc[-1]])
        ax.loglog(h_ref, g.l2_error.iloc[-1] * (h_ref / h_ref[-1]) ** p1, ":",
                  color=line.get_color(), label=f"slope {p1}")
    ax.set_xlabel("element size h")
    ax.set_ylabel("L2 error")
    ax.set_title("h-convergence")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend()
    fig.tight_layout()
    fig.savefig(figures / "h_convergence.png", dpi=150)
    plt.close(fig)

    out = pd.concat(rows)
    write_table(out[["npts", "nx", "h", "dofs", "iters_done", "l2_error", "l2_rate",
                     "linf_error"]].rename(columns={"iters_done": "cg_iters"}),
                tables, "h_convergence")


def timing_summary(df):
    """Median over repetitions of the max-over-ranks times, per rank count."""
    df = df.copy()
    df["t_per_iter"] = df["t_solve_max"] / df["n_matvec"]
    df["imbalance"] = df["t_kronecker_max"] / df["t_kronecker_avg"]
    cols = ["t_setup_max", "t_solve_max", "t_matvec_max", "t_kronecker_max",
            "t_gather_scatter_max", "t_allreduce_max", "t_per_iter", "imbalance",
            "iters_done", "dofs"]
    g = df.groupby("ranks")[cols].median().reset_index()
    g["reps"] = df.groupby("ranks").size().values
    g["other_matvec"] = (g.t_matvec_max - g.t_kronecker_max - g.t_gather_scatter_max).clip(lower=0)
    g["other_cg"] = (g.t_solve_max - g.t_matvec_max - g.t_allreduce_max).clip(lower=0)
    return g.sort_values("ranks").reset_index(drop=True)


def breakdown_plot(g, figures, name, title):
    parts = [("t_kronecker_max", "Kronecker products"),
             ("t_gather_scatter_max", "gather-scatter (gslib)"),
             ("t_allreduce_max", "Allreduce"),
             ("other_matvec", "rest of matvec"),
             ("other_cg", "rest of CG")]
    x = np.arange(len(g))
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))
    bottom = np.zeros(len(g))
    frac_bottom = np.zeros(len(g))
    total = sum(g[c].values for c, _ in parts)
    for col, label in parts:
        axes[0].bar(x, g[col], bottom=bottom, label=label)
        axes[1].bar(x, g[col] / total, bottom=frac_bottom, label=label)
        bottom += g[col].values
        frac_bottom += (g[col] / total).values
    for ax in axes:
        ax.set_xticks(x, g["ranks"].astype(str))
        ax.set_xlabel("MPI ranks")
    axes[0].set_ylabel("solve time [s] (max over ranks)")
    axes[1].set_ylabel("fraction of solve time")
    axes[0].set_title(title)
    axes[1].set_title(title + " (fractions)")
    axes[1].legend(loc="lower left", fontsize=8)
    fig.tight_layout()
    fig.savefig(figures / f"{name}_breakdown.png", dpi=150)
    plt.close(fig)


def strong_scaling(df, figures, tables):
    g = timing_summary(df)
    p0, t0 = g.ranks.iloc[0], g.t_solve_max.iloc[0]
    g["speedup"] = p0 * t0 / g.t_solve_max
    g["efficiency"] = g.speedup / g.ranks

    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))
    axes[0].loglog(g.ranks, g.speedup, "o-", label="measured")
    axes[0].loglog(g.ranks, g.ranks / p0, "k--", label="ideal")
    axes[0].set_ylabel(f"speedup (relative to {p0} rank(s))")
    axes[1].semilogx(g.ranks, g.efficiency, "o-")
    axes[1].axhline(1.0, color="k", ls="--")
    axes[1].set_ylabel("parallel efficiency")
    axes[1].set_ylim(0, 1.2)
    for ax in axes:
        ax.set_xlabel("MPI ranks")
        ax.grid(True, which="both", alpha=0.3)
    axes[0].legend()
    axes[0].set_title(f"strong scaling ({int(g.dofs.iloc[0])} dofs)")
    fig.tight_layout()
    fig.savefig(figures / "strong_scaling.png", dpi=150)
    plt.close(fig)
    breakdown_plot(g, figures, "strong_scaling", "strong scaling")

    write_table(g[["ranks", "reps", "dofs", "iters_done", "t_setup_max", "t_solve_max",
                   "t_per_iter", "speedup", "efficiency", "t_kronecker_max",
                   "t_gather_scatter_max", "t_allreduce_max", "imbalance"]]
                .rename(columns={"iters_done": "cg_iters"}), tables, "strong_scaling")


def weak_scaling(df, figures, tables):
    g = timing_summary(df)
    g["efficiency_total"] = g.t_solve_max.iloc[0] / g.t_solve_max
    g["efficiency_per_iter"] = g.t_per_iter.iloc[0] / g.t_per_iter

    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))
    axes[0].semilogx(g.ranks, g.t_per_iter, "o-", label="time per CG iteration")
    axes[0].set_ylabel("time per CG iteration [s]")
    axes[1].semilogx(g.ranks, g.efficiency_per_iter, "o-", label="per iteration")
    axes[1].semilogx(g.ranks, g.efficiency_total, "s--", label="total solve")
    axes[1].axhline(1.0, color="k", ls="--")
    axes[1].set_ylabel("weak scaling efficiency")
    axes[1].set_ylim(0, 1.2)
    axes[1].legend()
    for ax in axes:
        ax.set_xlabel("MPI ranks")
        ax.grid(True, which="both", alpha=0.3)
    per_rank = int(g.dofs.iloc[0])
    axes[0].set_title(f"weak scaling (~{per_rank} dofs per rank)")
    fig.tight_layout()
    fig.savefig(figures / "weak_scaling.png", dpi=150)
    plt.close(fig)
    breakdown_plot(g, figures, "weak_scaling", "weak scaling")

    write_table(g[["ranks", "reps", "dofs", "iters_done", "t_setup_max", "t_solve_max",
                   "t_per_iter", "efficiency_per_iter", "efficiency_total",
                   "t_kronecker_max", "t_gather_scatter_max", "t_allreduce_max",
                   "imbalance"]].rename(columns={"iters_done": "cg_iters"}),
                tables, "weak_scaling")


def main():
    results = Path(sys.argv[1] if len(sys.argv) > 1 else "results")
    figures, tables = results / "figures", results / "tables"
    figures.mkdir(parents=True, exist_ok=True)
    tables.mkdir(parents=True, exist_ok=True)

    studies = {"p_convergence": p_convergence, "h_convergence": h_convergence,
               "strong_scaling": strong_scaling, "weak_scaling": weak_scaling}
    found = False
    for study, func in studies.items():
        df = load(results, study)
        if df.empty:
            print(f"(no results for {study}, skipped)")
            continue
        found = True
        func(df, figures, tables)
    if not found:
        sys.exit(f"no results found in {results}")
    print(f"\nfigures: {figures}\ntables:  {tables}")


if __name__ == "__main__":
    main()
