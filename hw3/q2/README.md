# hw3 / q2: BLAS3 blocked parallel LU

Q2 code, kept separate from Q1 (`../src/`, untouched). Self-contained copy of the
Q1 build/run tooling plus the blocked factorization. Branch: `q2-blocked`.

| File               | Contents                                                          |
|--------------------|-------------------------------------------------------------------|
| `factor_blocked.c` | `lu_blocked`, local gemm/trsm kernels, driver (same as Q1's)      |
| `layout.c/.h`      | block-cyclic layout (copy of Q1's)                                |
| `make_cases.py`    | writes `cases/q2_*.txt`                                           |
| `plot_results.py`  | parses logs -> `results.csv`, `q2_*.png`, best-b and 80% tables   |
| `run_all.sh`       | build, submit all sweeps, then a dependent plot job (`plot.sbatch`, loads a python module; override with `PYTHON_MODULE=`) |

## Algorithm
Distribution block = pivot block = b (`nb` on stdin). For each diagonal block:
factor A11 (owner), broadcast it along its process row/column; `dtrsm` gives
L21 = A21 U11^-1 and U12 = L11^-1 A12; L21 is broadcast across process rows and U12
down process columns; every rank does `dgemm` A22 -= L21 U12. b=1 matches Q1.
Without BLAS (`make BLAS=none`) plain loops replace `cblas_dgemm/dtrsm`.
The stored factor is standard L (unit diagonal implied) + U; the 11x12 print test
negates L to match Q1's -L output.

## Use
    module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
    make
    ./run_all.sh                         # submits everything; plots when done

Run interface is the same as Q1: `./factor_blocked m n [P Q]` with b on stdin;
prints `RESULT m n b P Q nranks time gflops gflops_per_rank rel_err`.
Check `cases/q2_verify.txt` results first (rel_err ~1e-14).

Experiments: `q2_bsweep` (b = 4..128, n = 4000/8000, 1/16/64 ranks), strong and weak
scaling at b = 32, 64, 128 (strong n = 10000, 20000, 40000; weak n²/PQ = 4M, 16M, 64M).
