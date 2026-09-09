#!/usr/bin/env python3
"""Figures for Q2 from ping-pong CSVs.

  2a  one log-log figure per configuration: 1/2-round-trip time vs message
      size, one thin line per partner rank p, coloured by whether p is on the
      same node as rank 0, with the per-class median drawn over the top and the
      latency / m_2 / eager-limit guides annotated.

  2b  one comparison figure: the per-class median curve of every configuration
      on a shared axis, faceted intra-node | inter-node.

Needs numpy + matplotlib:

    python3 -m venv .venv
    .venv/bin/pip install -r analysis/requirements.txt
    .venv/bin/python analysis/plot_pingpong.py data/pp_P64_n1.csv

The numeric answers come from pingpong_stats.py, which needs no dependencies.
"""

import argparse
import os
import sys

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pingpong_stats import analyze, read_csv  # noqa: E402

# Chart chrome and ink, light surface (these figures are printed into a LaTeX
# report, so there is a single surface and no dark variant).
SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK_SECONDARY = "#52514e"
INK_MUTED = "#898781"
GRID = "#e1e0d9"
AXIS = "#c3c2b7"

# Categorical slots 1-3, in the documented order.  Identity is the message
# path, not the rank: 255 ranks are 3 series, not 255.
CLASS_COLOR = {"intra": "#2a78d6", "inter": "#eb6834", "self": "#1baf7a"}
CLASS_LABEL = {
    "intra": "same node",
    "inter": "across nodes",
    "self": "rank 0 to itself",
}
CLASS_ORDER = ("self", "intra", "inter")

# Categorical slots 1-5 for the configuration comparison (line charts use the
# adjacent pairlist, which this order clears).
CONFIG_COLORS = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"]


def style_axes(ax, xlabel, ylabel):
    ax.set_facecolor(SURFACE)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.grid(True, which="major", color=GRID, linewidth=0.6, zorder=0)
    ax.grid(True, which="minor", color=GRID, linewidth=0.3, alpha=0.6, zorder=0)
    ax.set_axisbelow(True)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(AXIS)
        ax.spines[side].set_linewidth(0.8)
    ax.tick_params(colors=INK_MUTED, labelsize=8, width=0.8)
    ax.set_xlabel(xlabel, color=INK_SECONDARY, fontsize=9)
    ax.set_ylabel(ylabel, color=INK_SECONDARY, fontsize=9)


def add_byte_axis(ax):
    """Top axis in bytes -- the same variable, rescaled, not a second scale."""
    top = ax.secondary_xaxis("top", functions=(lambda m: 8 * m, lambda b: b / 8))
    top.set_xlabel("message size (bytes)", color=INK_MUTED, fontsize=8)
    top.tick_params(colors=INK_MUTED, labelsize=7.5, width=0.8)
    top.spines["top"].set_color(AXIS)
    top.spines["top"].set_linewidth(0.8)
    return top


def edge_labels(ax, items, fontsize=8.5, min_gap=0.045):
    """Direct-label series at the right edge, nudged apart so none overlap.

    Curves that end at nearly the same height would otherwise print on top of
    each other.  Positions are worked out in axes fractions, then spread to a
    minimum vertical gap while keeping the original top-to-bottom order.
    """
    if not items:
        return
    y0, y1 = ax.get_ylim()
    span = np.log10(y1) - np.log10(y0)

    placed = []
    for y, text, color in items:
        frac = (np.log10(y) - np.log10(y0)) / span if span > 0 else 0.5
        placed.append([float(np.clip(frac, 0.0, 1.0)), text, color])
    placed.sort(key=lambda p: p[0])

    for i in range(1, len(placed)):  # push up
        if placed[i][0] - placed[i - 1][0] < min_gap:
            placed[i][0] = placed[i - 1][0] + min_gap
    overflow = placed[-1][0] - 1.0
    if overflow > 0:  # then slide the whole stack back into range
        for p in placed:
            p[0] -= overflow
        for i in range(len(placed) - 2, -1, -1):
            if placed[i + 1][0] - placed[i][0] < min_gap:
                placed[i][0] = placed[i + 1][0] - min_gap

    for frac, text, color in placed:
        ax.annotate(text, xy=(1.0, frac), xycoords="axes fraction",
                    xytext=(6, 0), textcoords="offset points",
                    color=color, fontsize=fontsize, va="center", zorder=6,
                    annotation_clip=False)


def class_median(curves, cls):
    """Median t(m) across all partners in one class, on the shared size grid."""
    members = [c for c in curves if c.cls == cls]
    if not members:
        return None, None
    grid = members[0].m
    stack = np.array([c.t for c in members if c.m == grid], dtype=float)
    if stack.size == 0:
        return None, None
    return np.asarray(grid, dtype=float), np.median(stack, axis=0)


def median_of(rows, cls, key):
    vals = [r[key] for r in rows if r["class"] == cls and r[key] is not None]
    return float(np.median(vals)) if vals else None


def figure_2a(meta, curves, rows, out_base, title):
    fig, ax = plt.subplots(figsize=(7.2, 4.8), dpi=150)
    fig.patch.set_facecolor(SURFACE)
    style_axes(ax, "message size, m (64-bit words)",
               "1/2 round-trip time (microseconds)")

    present = [c for c in CLASS_ORDER if any(cu.cls == c for cu in curves)]

    # Every partner rank, thin and recessive: the spread is the message.
    for cu in curves:
        ax.plot(cu.m, np.array(cu.t) * 1e6, color=CLASS_COLOR[cu.cls],
                linewidth=0.5, alpha=0.16, zorder=2, solid_capstyle="round")

    # The per-class median carries the reading.
    ends = []
    for cls in present:
        m, t = class_median(curves, cls)
        if m is None:
            continue
        n = sum(1 for cu in curves if cu.cls == cls)
        ax.plot(m, t * 1e6, color=CLASS_COLOR[cls], linewidth=2.0, zorder=5,
                solid_capstyle="round",
                label="%s (%d rank%s)" % (CLASS_LABEL[cls], n,
                                          "" if n == 1 else "s"))
        ends.append((t[-1] * 1e6, CLASS_LABEL[cls], CLASS_COLOR[cls]))

    # Three series, so all get a direct label as well as the legend.
    edge_labels(ax, ends)

    # Guides, drawn from the same numbers the summary table reports.
    ref_cls = "inter" if "inter" in present else ("intra" if "intra" in present
                                                  else present[0])
    lat = median_of(rows, ref_cls, "t_lat_s")
    m2 = median_of(rows, ref_cls, "m2_words")
    eager = median_of(rows, ref_cls, "m_eager_words")

    ref_name = CLASS_LABEL[ref_cls]
    if lat:
        ax.axhline(lat * 1e6, color=INK_MUTED, linewidth=0.8, linestyle=(0, (4, 3)),
                   zorder=3)
        ax.annotate("%s: latency %.2f us" % (ref_name, lat * 1e6),
                    xy=(curves[0].m[0], lat * 1e6), xytext=(2, 4),
                    textcoords="offset points", color=INK_SECONDARY, fontsize=8)

    # m_2 and the eager limit often sit close together on the x axis, so they
    # are annotated at opposite ends of the y axis to keep the labels apart.
    lo, hi = ax.get_ylim()
    guides = []
    if m2:
        guides.append((m2, "$m_2$ = %d words" % round(m2), hi, (4, -6), "top"))
    if eager:
        guides.append((eager, "eager limit %d words (%d B)"
                       % (round(eager), 8 * round(eager)), lo, (4, 6), "bottom"))
    for value, text, y, offset, va in guides:
        ax.axvline(value, color=INK_MUTED, linewidth=0.8, linestyle=(0, (1, 3)),
                   zorder=3)
        ax.annotate(text, xy=(value, y), xytext=offset,
                    textcoords="offset points", color=INK_SECONDARY, fontsize=8,
                    va=va)
    ax.set_ylim(lo, hi)

    add_byte_axis(ax)
    ax.set_title(title, color=INK, fontsize=11, fontweight="bold",
                 loc="left", pad=26)

    leg = ax.legend(loc="upper left", frameon=False, fontsize=8.5,
                    labelcolor=INK_SECONDARY)
    for text in leg.get_texts():
        text.set_color(INK_SECONDARY)

    fig.tight_layout()
    save(fig, out_base)


def figure_2b(datasets, out_base):
    """Class medians of every configuration, faceted intra | inter."""
    panels = [c for c in ("intra", "inter")
              if any(any(cu.cls == c for cu in d["curves"]) for d in datasets)]
    if not panels:
        print("nothing to compare", file=sys.stderr)
        return

    fig, axes = plt.subplots(1, len(panels), figsize=(4.6 * len(panels), 4.6),
                             dpi=150, squeeze=False)
    fig.patch.set_facecolor(SURFACE)

    for k, cls in enumerate(panels):
        ax = axes[0][k]
        style_axes(ax, "message size, m (64-bit words)",
                   "1/2 round-trip time (microseconds)" if k == 0 else "")
        ends = []
        for i, d in enumerate(datasets):
            m, t = class_median(d["curves"], cls)
            if m is None:
                continue
            color = CONFIG_COLORS[i % len(CONFIG_COLORS)]
            ax.plot(m, t * 1e6, color=color, linewidth=1.8, zorder=5,
                    solid_capstyle="round", label=d["label"])
            ends.append((t[-1] * 1e6, d["label"], color))
        edge_labels(ax, ends, fontsize=7.5, min_gap=0.04)
        add_byte_axis(ax)
        ax.set_title(CLASS_LABEL[cls], color=INK, fontsize=10,
                     fontweight="bold", loc="left", pad=26)
        if k > 0:
            ax.set_ylabel("")

    handles, labels = axes[0][0].get_legend_handles_labels()
    leg = fig.legend(handles, labels, loc="lower center", ncol=len(labels),
                     frameon=False, fontsize=8.5, bbox_to_anchor=(0.5, -0.02))
    for text in leg.get_texts():
        text.set_color(INK_SECONDARY)

    fig.suptitle("Ping-pong scaling with rank count", color=INK, fontsize=11,
                 fontweight="bold", x=0.01, ha="left")
    fig.tight_layout(rect=(0, 0.05, 1, 0.96))
    save(fig, out_base)


def save(fig, out_base):
    os.makedirs(os.path.dirname(out_base) or ".", exist_ok=True)
    for ext in ("png", "pdf"):
        path = "%s.%s" % (out_base, ext)
        fig.savefig(path, facecolor=SURFACE, bbox_inches="tight")
        print("wrote %s" % path)
    plt.close(fig)


def label_for(meta, path):
    P = meta.get("P")
    nodes = meta.get("nodes")
    if P and nodes:
        return "P=%s, %s node%s" % (P, nodes, "" if nodes == "1" else "s")
    return os.path.splitext(os.path.basename(path))[0]


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv", nargs="+", help="ping-pong result CSV(s)")
    ap.add_argument("--fig-dir", default="../report/figures",
                    help="output directory (default: %(default)s)")
    ap.add_argument("--no-compare", action="store_true",
                    help="skip the cross-configuration figure")
    args = ap.parse_args()

    datasets = []
    for path in args.csv:
        meta, curves = read_csv(path)
        if not curves:
            print("%s: no usable rows" % path, file=sys.stderr)
            continue
        rows = [analyze(c) for c in curves]
        tag = os.path.splitext(os.path.basename(path))[0]
        label = label_for(meta, path)
        datasets.append({"meta": meta, "curves": curves, "rows": rows,
                         "tag": tag, "label": label})
        figure_2a(meta, curves, rows,
                  os.path.join(args.fig_dir, tag),
                  "Half round-trip time from rank 0  --  %s" % label)

    if len(datasets) > 1 and not args.no_compare:
        figure_2b(datasets, os.path.join(args.fig_dir, "pp_compare"))


if __name__ == "__main__":
    main()
