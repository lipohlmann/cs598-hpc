#!/usr/bin/env python3
"""Write Q2 (blocked LU) sweep case files for sweep.sbatch into cases/.

    python3 make_cases.py

Each line: INPUT=<b> <nranks> ./factor_blocked m n P Q
b is both the distribution block size and the pivot block size.
"""
import math
import os

NPS = [1, 2, 4, 8, 16, 32, 64, 128]
BS_SWEEP = [4, 8, 16, 20, 32, 48, 64, 96, 128]
BS_SCALE = [32, 64, 128]                       # block sizes used for scaling runs
SWEEP_N = [4000, 8000]
SWEEP_NP = [1, 16, 64]
STRONG_N = [10000, 20000, 40000]
WEAK_LOCAL = {"4M": 4_000_000, "16M": 16_000_000, "64M": 64_000_000}  # n^2/(P Q)


def grid(np_):
    """Most square P x Q with P <= Q."""
    p = max(d for d in range(1, int(math.isqrt(np_)) + 1) if np_ % d == 0)
    return p, np_ // p


def line(np_, n, b, pq=None):
    p, q = pq or grid(np_)
    return f"INPUT={b} {np_} ./factor_blocked {n} {n + 1} {p} {q}"


def write(name, header, lines):
    path = os.path.join("cases", name)
    with open(path, "w") as f:
        f.write(f"# {header}\n" + "\n".join(lines) + "\n")
    print(f"wrote {path} ({len(lines)} runs)")


os.makedirs("cases", exist_ok=True)

write("q2_bsweep.txt", "block size sweep: b x n x ranks",
      [line(np_, n, b) for n in SWEEP_N for np_ in SWEEP_NP for b in BS_SWEEP])

for b in BS_SCALE:
    write(f"q2_strong_b{b}.txt", f"strong scaling, b={b}",
          [line(np_, n, b) for n in STRONG_N for np_ in NPS])
    for tag, loc in WEAK_LOCAL.items():
        write(f"q2_weak_{tag}_b{b}.txt", f"weak scaling, n^2/(P Q)={loc}, b={b}",
              [line(np_, round(math.sqrt(loc * np_)), b) for np_ in NPS])

verify = []
for np_, p, q in [(1, 1, 1), (4, 2, 2), (4, 4, 1), (4, 1, 4), (6, 2, 3), (6, 3, 2), (12, 3, 4)]:
    for n in (97, 300):
        for b in (1, 3, 8, 20, 64, 128):
            verify.append(line(np_, n, b, (p, q)))
write("q2_verify.txt", "correctness: rel_err in RESULT lines should be ~1e-14", verify)
