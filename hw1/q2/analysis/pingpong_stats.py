#!/usr/bin/env python3
"""Extract the Q2 answers from ping-pong CSVs.

Per partner rank p this computes the four quantities the assignment asks for:

  latency            t(m=1), seconds
  inverse bandwidth  seconds per 64-bit word as m grows large
  m_2                the m where t(m_2) = 2 t(1)
  eager limit        the largest m still sent eagerly

Standard library only -- no numpy, no matplotlib -- so this runs anywhere,
including a cluster login node.  Plotting lives in plot_pingpong.py.

usage:  python3 pingpong_stats.py data/pp_P64_n1.csv [more.csv ...]
"""

import argparse
import csv
import math
import os
import statistics
import sys

# Fit the inverse bandwidth over the top decade of message sizes.
LARGE_M_FRACTION = 0.1

# Restrict eager-limit detection to sizes where a protocol switch is plausible.
EAGER_M_MIN = 64
EAGER_M_MAX_FRACTION = 0.5

CLASS_ORDER = ("self", "intra", "inter")


class Curve:
    """One partner's t(m) curve."""

    def __init__(self, partner, host, cls):
        self.partner = partner
        self.host = host
        self.cls = cls
        self.m = []
        self.t = []

    def add(self, m, t):
        self.m.append(m)
        self.t.append(t)

    def sort(self):
        pairs = sorted(zip(self.m, self.t))
        self.m = [p[0] for p in pairs]
        self.t = [p[1] for p in pairs]


def read_csv(path):
    """Return (metadata dict, list of Curve) for one results file."""
    meta = {"file": os.path.basename(path)}
    curves = {}

    with open(path, newline="") as fh:
        lines = []
        for line in fh:
            if line.startswith("#"):
                for tok in line.lstrip("#").split():
                    if "=" in tok:
                        k, v = tok.split("=", 1)
                        meta[k] = v
            else:
                lines.append(line)

        for row in csv.DictReader(lines):
            p = int(row["partner"])
            if p not in curves:
                curves[p] = Curve(p, row["partner_host"], row["class"])
            t = float(row["t_half_s"])
            if t > 0.0:  # partner rows never carry a negative time, but be safe
                curves[p].add(int(row["nwds"]), t)

    out = []
    for p in sorted(curves):
        curves[p].sort()
        if curves[p].m:
            out.append(curves[p])
    return meta, out


def median3(y):
    """Median-of-three smoothing, used only for jump detection."""
    if len(y) < 3:
        return list(y)
    s = [y[0]]
    s += [statistics.median(y[i - 1 : i + 2]) for i in range(1, len(y) - 1)]
    s.append(y[-1])
    return s


def lsq_slope(x, y):
    """Least-squares slope of y = a + b x.  Returns b, or None if degenerate."""
    n = len(x)
    if n < 2:
        return None
    sx = sum(x)
    sy = sum(y)
    sxx = sum(v * v for v in x)
    sxy = sum(a * b for a, b in zip(x, y))
    den = n * sxx - sx * sx
    if den == 0.0:
        return None
    return (n * sxy - sx * sy) / den


def inverse_bandwidth(c):
    """Slope of t vs m over the largest sizes: seconds per word."""
    m_cut = c.m[-1] * LARGE_M_FRACTION
    xs = [float(m) for m in c.m if m >= m_cut]
    ys = [t for m, t in zip(c.m, c.t) if m >= m_cut]
    if len(xs) < 3:
        return None
    b = lsq_slope(xs, ys)
    if b is None or b <= 0.0:
        return None
    return b


def m_half(c, t_ref, ts):
    """Smallest m with t(m) >= 2 t_ref, interpolated in log m.

    Uses the smoothed curve: a single noisy point in the latency-flat region is
    otherwise enough to trigger a false crossing hundreds of words early.
    """
    target = 2.0 * t_ref
    for i in range(1, len(c.m)):
        if ts[i] >= target > ts[i - 1]:
            t0, t1 = ts[i - 1], ts[i]
            m0, m1 = c.m[i - 1], c.m[i]
            if t1 == t0 or m0 <= 0:
                return float(m1)
            f = (target - t0) / (t1 - t0)
            return math.exp(math.log(m0) + f * (math.log(m1) - math.log(m0)))
    return None


def eager_limit(c, ts):
    """Largest m still sent eagerly.

    A protocol switch shows up as a step in t that is larger than the steady
    growth explained by the message size itself.  Score each consecutive pair by
    how much the time jump exceeds the size jump in log space and take the
    biggest.  Smoothed, since a single noisy point would otherwise win.

    Per partner this is only as good as the data: on a noisy or oversubscribed
    machine an unrelated later jump can outscore the real protocol step.  The
    median across the ranks in a class is the robust number -- report() prints
    it with the full [min, max] spread so that disagreement stays visible.
    """
    if len(c.m) < 4:
        return None, None
    m_hi = c.m[-1] * EAGER_M_MAX_FRACTION

    best_score, best_m = 0.0, None
    for i in range(len(c.m) - 1):
        if not (EAGER_M_MIN <= c.m[i] <= m_hi):
            continue
        if ts[i] <= 0.0 or ts[i + 1] <= 0.0 or c.m[i] <= 0:
            continue
        score = math.log(ts[i + 1] / ts[i]) - math.log(c.m[i + 1] / c.m[i])
        if score > best_score:
            best_score, best_m = score, c.m[i]
    return best_m, (best_score if best_m else None)


def analyze(c):
    """All four quantities for one partner."""
    ts = median3(c.t)

    t1 = c.t[0] if c.m[0] == 1 else None
    small = [t for m, t in zip(c.m, c.t) if m <= 4]
    t_lat_min = min(small) if small else None
    ref = t1 if t1 is not None else t_lat_min

    b = inverse_bandwidth(c)
    m2 = m_half(c, ref, ts) if ref else None
    m_eager, score = eager_limit(c, ts)

    return {
        "partner": c.partner,
        "host": c.host,
        "class": c.cls,
        "t_lat_s": t1,
        "t_lat_min_s": t_lat_min,
        "inv_bw_s_per_word": b,
        "bw_GB_s": (8.0 / b / 1e9) if b else None,
        "m2_words": m2,
        "m_eager_words": m_eager,
        "m_eager_bytes": (8 * m_eager) if m_eager else None,
        "eager_score": score,
    }


def fmt(v, kind):
    if v is None:
        return "--"
    if kind == "us":
        return "%.3f" % (v * 1e6)
    if kind == "ns_word":
        return "%.4f" % (v * 1e9)
    if kind == "gb":
        return "%.2f" % v
    if kind == "int":
        return "%d" % round(v)
    return str(v)


def summarize(rows, key):
    vals = [r[key] for r in rows if r[key] is not None]
    if not vals:
        return None, None, None
    return min(vals), statistics.median(vals), max(vals)


COLUMNS = [
    ("t_lat_s", "latency (us)", "us"),
    ("inv_bw_s_per_word", "inv bw (ns/word)", "ns_word"),
    ("bw_GB_s", "bandwidth (GB/s)", "gb"),
    ("m2_words", "m_2 (words)", "int"),
    ("m_eager_words", "eager limit (words)", "int"),
    ("m_eager_bytes", "eager limit (bytes)", "int"),
]


def report(meta, rows, out_dir, tag, quiet=False):
    by_class = {}
    for r in rows:
        by_class.setdefault(r["class"], []).append(r)

    if not quiet:
        print("=" * 78)
        print("%s   P=%s nodes=%s msg_vol=%s" % (meta.get("file", tag),
                                                 meta.get("P", "?"),
                                                 meta.get("nodes", "?"),
                                                 meta.get("msg_vol", "?")))
        print("=" * 78)
        head = "%-7s %5s  " % ("class", "n") + "".join(
            "%28s" % c[1] for c in COLUMNS[:4]
        )
        print(head)
        print("-" * len(head))
        for cls in CLASS_ORDER:
            if cls not in by_class:
                continue
            group = by_class[cls]
            line = "%-7s %5d  " % (cls, len(group))
            for key, _, kind in COLUMNS[:4]:
                lo, med, hi = summarize(group, key)
                line += "%28s" % (
                    "--" if med is None
                    else "%s [%s,%s]" % (fmt(med, kind), fmt(lo, kind), fmt(hi, kind))
                )
            print(line)
        print()
        for cls in CLASS_ORDER:
            if cls not in by_class:
                continue
            lo, med, hi = summarize(by_class[cls], "m_eager_bytes")
            m2lo, m2med, m2hi = summarize(by_class[cls], "m2_words")
            print("  %-5s  eager limit  median %s B  [%s, %s]     "
                  "m_2 median %s words [%s, %s]"
                  % (cls, fmt(med, "int"), fmt(lo, "int"), fmt(hi, "int"),
                     fmt(m2med, "int"), fmt(m2lo, "int"), fmt(m2hi, "int")))
        print()

    os.makedirs(out_dir, exist_ok=True)

    csv_path = os.path.join(out_dir, "summary_%s.csv" % tag)
    with open(csv_path, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        w.writeheader()
        for r in rows:
            w.writerow(r)

    tex_path = os.path.join(out_dir, "summary_%s.tex" % tag)
    with open(tex_path, "w") as fh:
        fh.write("%% generated by analysis/pingpong_stats.py from %s\n"
                 % meta.get("file", tag))
        fh.write("\\begin{tabular}{lrrrrr}\n\\toprule\n")
        fh.write("class & ranks & latency ($\\mu$s) & inv.\\ bw (ns/word) & "
                 "$m_2$ (words) & eager (bytes) \\\\\n\\midrule\n")
        for cls in CLASS_ORDER:
            if cls not in by_class:
                continue
            group = by_class[cls]
            cells = [cls, str(len(group))]
            for key, _, kind in (COLUMNS[0], COLUMNS[1], COLUMNS[3], COLUMNS[5]):
                _, med, _ = summarize(group, key)
                cells.append(fmt(med, kind))
            fh.write(" & ".join(cells) + " \\\\\n")
        fh.write("\\bottomrule\n\\end{tabular}\n")

    if not quiet:
        print("wrote %s" % csv_path)
        print("wrote %s" % tex_path)
    return csv_path, tex_path


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv", nargs="+", help="ping-pong result CSV(s)")
    ap.add_argument("--out-dir", default="analysis/out",
                    help="where summary_*.csv / .tex are written")
    ap.add_argument("--per-partner", action="store_true",
                    help="also print one line per partner rank")
    args = ap.parse_args()

    for path in args.csv:
        meta, curves = read_csv(path)
        if not curves:
            print("%s: no usable rows" % path, file=sys.stderr)
            continue
        rows = [analyze(c) for c in curves]
        tag = os.path.splitext(os.path.basename(path))[0]
        report(meta, rows, args.out_dir, tag)

        if args.per_partner:
            print("%-8s %-12s %-6s %10s %12s %10s %12s"
                  % ("partner", "host", "class", "lat(us)", "invbw(ns/w)",
                     "m_2", "eager(B)"))
            for r in rows:
                print("%-8d %-12s %-6s %10s %12s %10s %12s"
                      % (r["partner"], r["host"][:12], r["class"],
                         fmt(r["t_lat_s"], "us"),
                         fmt(r["inv_bw_s_per_word"], "ns_word"),
                         fmt(r["m2_words"], "int"),
                         fmt(r["m_eager_bytes"], "int")))
            print()


if __name__ == "__main__":
    main()
