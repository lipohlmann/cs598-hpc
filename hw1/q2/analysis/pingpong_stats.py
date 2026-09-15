#!/usr/bin/env python3
"""Extract the Q2 answers from ping-pong CSVs.

Per partner rank p this computes the four quantities the assignment asks for:

  latency            t(m=1), seconds
  inverse bandwidth  seconds per 64-bit word as m grows large
  m_2                the m where t(m_2) = 2 t(1)
  eager limit        the largest m still sent eagerly

Results are summarized per (class, socket): self / same node / across nodes,
split by the socket the partner ran on.  Slurm's default task distribution
puts consecutive ranks on alternating sockets, and the two sockets measure
very differently (up to 3x at large m across nodes), so a plain per-class
median would mix two populations.  CSVs written before the partner_socket
column existed fall back to the plain class.

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

# The inverse bandwidth is t/m over the largest sizes.  A slope fit over the
# top decade is NOT usable here: a second protocol switch sits at 200-400 KB,
# where t roughly halves, so a line through that range means nothing (the
# first version of this script fit one and reported 90+ GB/s for a self-send).
LARGE_M_NPTS = 5

# Restrict eager-limit detection to sizes where a protocol switch is plausible.
EAGER_M_MIN = 64
EAGER_M_MAX_FRACTION = 0.5

CLASS_ORDER = ("self", "intra", "inter")


class Curve:
    """One partner's t(m) curve."""

    def __init__(self, partner, host, cls, socket):
        self.partner = partner
        self.host = host
        self.cls = cls
        self.socket = socket  # None if the CSV predates the column
        self.m = []
        self.t = []

    def add(self, m, t):
        self.m.append(m)
        self.t.append(t)

    def sort(self):
        pairs = sorted(zip(self.m, self.t))
        self.m = [p[0] for p in pairs]
        self.t = [p[1] for p in pairs]


def socket_of(v):
    """partner_socket column -> int, or None if absent or unknown (-1)."""
    if v is None or v == "":
        return None
    s = int(v)
    return s if s >= 0 else None


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
                curves[p] = Curve(p, row["partner_host"], row["class"],
                                  socket_of(row.get("partner_socket")))
            t = float(row["t_half_s"])
            if t > 0.0:  # partner rows never carry a negative time, but be safe
                curves[p].add(int(row["nwds"]), t)

    out = []
    for p in sorted(curves):
        curves[p].sort()
        if curves[p].m:
            out.append(curves[p])
    return meta, out


def group_name(cls, socket):
    return cls if socket is None else "%s/s%d" % (cls, socket)


def bucket(items, key):
    """Bucket curves or result rows by (class, socket), in display order."""
    out = {}
    for it in items:
        out.setdefault(key(it), []).append(it)

    def order(k):
        cls, s = k
        return (CLASS_ORDER.index(cls), -1 if s is None else s)

    return [(k, out[k]) for k in sorted(out, key=order)]


def median3(y):
    """Median-of-three smoothing, used only for jump detection."""
    if len(y) < 3:
        return list(y)
    s = [y[0]]
    s += [statistics.median(y[i - 1 : i + 2]) for i in range(1, len(y) - 1)]
    s.append(y[-1])
    return s


def inverse_bandwidth(c):
    """Median of t/m over the last LARGE_M_NPTS sizes: seconds per word.

    This includes the latency term, which is under 3% of t at 800 KB.
    """
    if len(c.m) < LARGE_M_NPTS:
        return None
    return statistics.median(
        t / m for m, t in zip(c.m[-LARGE_M_NPTS:], c.t[-LARGE_M_NPTS:]))


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
    """Largest m still sent eagerly, and the next tested size after it.

    A protocol switch shows up as a step in t that is larger than the steady
    growth explained by the message size itself.  Score each consecutive pair by
    how much the time jump exceeds the size jump in log space and take the
    biggest.  Smoothed, since a single noisy point would otherwise win.

    The true limit lies between the two sizes returned; the size grid is too
    coarse there (8088 and 8224 bytes) to say more than "8 KB".

    Per partner this is only as good as the data: on a noisy or oversubscribed
    machine an unrelated later jump can outscore the real protocol step.  The
    median across the ranks in a group is the robust number -- report() prints
    it with the full [min, max] spread so that disagreement stays visible.
    """
    if len(c.m) < 4:
        return None, None, None
    m_hi = c.m[-1] * EAGER_M_MAX_FRACTION

    best_score, best_i = 0.0, None
    for i in range(len(c.m) - 1):
        if not (EAGER_M_MIN <= c.m[i] <= m_hi):
            continue
        if ts[i] <= 0.0 or ts[i + 1] <= 0.0 or c.m[i] <= 0:
            continue
        score = math.log(ts[i + 1] / ts[i]) - math.log(c.m[i + 1] / c.m[i])
        if score > best_score:
            best_score, best_i = score, i
    if best_i is None:
        return None, None, None
    return c.m[best_i], c.m[best_i + 1], best_score


def analyze(c):
    """All four quantities for one partner."""
    ts = median3(c.t)

    t1 = c.t[0] if c.m[0] == 1 else None
    small = [t for m, t in zip(c.m, c.t) if m <= 4]
    t_lat_min = min(small) if small else None
    ref = t1 if t1 is not None else t_lat_min

    b = inverse_bandwidth(c)
    m2 = m_half(c, ref, ts) if ref else None
    m_eager, m_next, score = eager_limit(c, ts)

    return {
        "partner": c.partner,
        "host": c.host,
        "class": c.cls,
        "socket": c.socket,
        "t_lat_s": t1,
        "t_lat_min_s": t_lat_min,
        "inv_bw_s_per_word": b,
        "bw_GB_s": (8.0 / b / 1e9) if b else None,
        "m2_words": m2,
        "m_eager_words": m_eager,
        "m_eager_bytes": (8 * m_eager) if m_eager else None,
        "m_eager_next_bytes": (8 * m_next) if m_next else None,
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
]


def report(meta, rows, out_dir, tag, quiet=False):
    groups = bucket(rows, lambda r: (r["class"], r["socket"]))

    if not quiet:
        print("=" * 78)
        print("%s   P=%s nodes=%s msg_vol=%s rank0 socket=%s"
              % (meta.get("file", tag), meta.get("P", "?"),
                 meta.get("nodes", "?"), meta.get("msg_vol", "?"),
                 meta.get("socket0", "?")))
        print("=" * 78)
        head = "%-9s %5s  " % ("group", "n") + "".join(
            "%28s" % c[1] for c in COLUMNS
        )
        print(head)
        print("-" * len(head))
        for (cls, sock), group in groups:
            line = "%-9s %5d  " % (group_name(cls, sock), len(group))
            for key, _, kind in COLUMNS:
                lo, med, hi = summarize(group, key)
                line += "%28s" % (
                    "--" if med is None
                    else "%s [%s,%s]" % (fmt(med, kind), fmt(lo, kind), fmt(hi, kind))
                )
            print(line)
        print()
        for (cls, sock), group in groups:
            lo, med, hi = summarize(group, "m_eager_bytes")
            _, nxt, _ = summarize(group, "m_eager_next_bytes")
            m2lo, m2med, m2hi = summarize(group, "m2_words")
            print("  %-9s eager limit between %s and %s B  (last eager size "
                  "spread [%s, %s])   m_2 median %s words [%s, %s]"
                  % (group_name(cls, sock), fmt(med, "int"), fmt(nxt, "int"),
                     fmt(lo, "int"), fmt(hi, "int"),
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
        fh.write("group & ranks & latency ($\\mu$s) & inv.\\ bw (ns/word) & "
                 "$m_2$ (words) & eager limit (bytes) \\\\\n\\midrule\n")
        for (cls, sock), group in groups:
            cells = [group_name(cls, sock), str(len(group))]
            for key, _, kind in (COLUMNS[0], COLUMNS[1], COLUMNS[3]):
                _, med, _ = summarize(group, key)
                cells.append(fmt(med, kind))
            _, lo, _ = summarize(group, "m_eager_bytes")
            _, hi, _ = summarize(group, "m_eager_next_bytes")
            cells.append("%s--%s" % (fmt(lo, "int"), fmt(hi, "int")))
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
            print("%-8s %-12s %-6s %4s %10s %12s %10s %12s"
                  % ("partner", "host", "class", "sock", "lat(us)",
                     "invbw(ns/w)", "m_2", "eager(B)"))
            for r in rows:
                print("%-8d %-12s %-6s %4s %10s %12s %10s %12s"
                      % (r["partner"], r["host"][:12], r["class"],
                         "--" if r["socket"] is None else r["socket"],
                         fmt(r["t_lat_s"], "us"),
                         fmt(r["inv_bw_s_per_word"], "ns_word"),
                         fmt(r["m2_words"], "int"),
                         fmt(r["m_eager_bytes"], "int")))
            print()


if __name__ == "__main__":
    main()
