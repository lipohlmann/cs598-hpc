#!/usr/bin/env python3
"""Write sweep case files for sweep.sbatch into cases/.

    python3 make_cases.py            # strong.txt, weak_*.txt, verify.txt

Each line: INPUT=<nb> <nranks> ./factor_parallel m n P Q   (nb=1 for Q1)
"""
import math
import os

NPS = [1, 2, 4, 8, 16, 32, 64, 128]
STRONG_N = [1000, 2000, 4000]
WEAK_LOCAL = {"250k": 250_000, "1M": 1_000_000, "4M": 4_000_000}  # n^2/(P Q)


def grid(np_):
    """Most square P x Q with P <= Q."""
    p = max(d for d in range(1, int(math.isqrt(np_)) + 1) if np_ % d == 0)
    return p, np_ // p


def line(np_, n, nb=1, pq=None):
    p, q = pq or grid(np_)
    return f"INPUT={nb} {np_} ./factor_parallel {n} {n + 1} {p} {q}"


def write(name, header, lines):
    path = os.path.join("cases", name)
    with open(path, "w") as f:
        f.write(f"# {header}\n" + "\n".join(lines) + "\n")
    print(f"wrote {path} ({len(lines)} runs)")


os.makedirs("cases", exist_ok=True)

write("strong.txt", "strong scaling: n fixed, ranks increasing",
      [line(np_, n) for n in STRONG_N for np_ in NPS])

for tag, loc in WEAK_LOCAL.items():
    write(f"weak_{tag}.txt", f"weak scaling: n^2/(P Q) = {loc}",
          [line(np_, round(math.sqrt(loc * np_))) for np_ in NPS])

verify = []
for np_, p, q in [(1, 1, 1), (4, 2, 2), (4, 4, 1), (4, 1, 4), (6, 2, 3), (6, 3, 2), (12, 3, 4)]:
    for n in (97, 300):
        for nb in (1, 3, 8):
            verify.append(line(np_, n, nb, (p, q)))
write("verify.txt", "correctness: rel_err in RESULT lines should be ~1e-14", verify)
