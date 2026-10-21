# hw3 / src: build and test harness

Parallel LU factorization (no pivoting) on a 2D block-cyclic layout, plus
the build and run tooling for it. The tooling doesn't refer to any
specific source file, so it keeps working as the code changes.

## Code

| File                | Contents                                                       |
|---------------------|----------------------------------------------------------------|
| `layout.h`          | `Aij` column-major macro, `Layout` struct, helper prototypes   |
| `layout.c`          | block-cyclic layout helpers, test matrix, gather-and-print     |
| `factor_parallel.c` | `lu_unblocked` and a small test `main`                         |

### Layout

The m x n matrix is split into `nb x nb` blocks dealt round-robin over a
P x Q process grid. Rank r sits at grid position
`(myrow, mycol) = (r % P, r / P)` (column-major). With `nb = 1` this is the
interleaved layout from `lu.pdf`. Each rank stores only its own
`mloc x nloc` piece, column-major with `lda = max(1, mloc)`, and all code
indexes it with local indices.

| Helper                        | Purpose                                                    |
|-------------------------------|------------------------------------------------------------|
| `build_local(m,n,nb,P,Q,...)` | fills a `Layout`; aborts if `P*Q` != number of ranks       |
| `numroc(n,nb,iproc,nprocs)`   | how many of the n rows/cols process coordinate `iproc` owns (ScaLAPACK name) |
| `global_row/col`              | local index -> global index                                |
| `owner_row/col`               | global index -> owning process row/col                     |
| `local_row/col`               | global index -> local index (valid on the owner)           |
| `spd(a, lda, L)`              | fills my piece of the hw3 test matrix                      |
| `print_global(a,lda,name,L,comm)` | gathers to rank 0 and prints; all ranks call it; small sizes only |

`numroc(k+1, nb, myrow, P)` is also the first local row whose global index
is greater than k, which is how the factorization finds the start of its
trailing matrix.

### Test matrix

From `hw3.pdf`, with N = m + 1: A = S + N*I, where
s_ij = sqrt(2/N) sin(pi*i*j/N) for i, j = 1..m. S is symmetric and
orthogonal, so A is SPD (eigenvalues N +/- 1) and needs no pivoting.
Columns beyond m (the augmented `[A | b]` system from `lu.pdf`) get
b = A * ones, so the exact solution is all ones.

### Factorization

`lu_unblocked` is the BLAS1 right-looking algorithm from `lu.pdf`, one
column at a time for k = 0 .. min(m,n)-1:

1. The process row owning row k broadcasts it (columns >= k, so the
   diagonal entry a(k,k) goes along) down each process column.
2. The process column owning column k computes the multipliers
   a(i,k) <- -a(i,k)/a(k,k) for i > k.
3. It broadcasts them across each process row.
4. Every rank applies the rank-1 update to its trailing part.

The storage matches `../factor.c`: -L strictly below the diagonal (unit
diagonal implied), U on and above it. The algorithm is unblocked whatever
the value of `nb`; `nb` only sets the data distribution. Use `nb = 1` for Q1.

## Build

    module load openmpi amd/aocl-aocc/aocl-aocc-5.0.0
    make                 # builds every program in this directory
    make list            # shows what make thinks the programs/helpers are
    make clean

How sources are picked up:

- Every `.c` file with a line starting `int main` becomes an executable of
  the same name (`factor_parallel.c` -> `./factor_parallel`).
- Every other `.c` file is a helper and is linked into **all** programs,
  so put shared code (grid setup, matrix init, checks, ...) in its own
  file and declare it in a header here or in `../include/`.

Options:

| Variable | Default     | Meaning                                                   |
|----------|-------------|-----------------------------------------------------------|
| `BLAS`   | `blis`      | `blis` (AOCL), `mkl` (oneMKL), or `none`                  |
| `ARCH`   | `x86-64-v3` | `-march=` value; the default runs on every compute node   |
| `MPICC`  | `mpicc`     | compiler wrapper                                          |

For dgemm (Q2), use `#include <cblas.h>` and call `cblas_dgemm(...)`.
The same code builds with either library:

    make BLAS=blis                                    # default
    module load intel/tbb intel/compiler-rt intel/mkl
    make clean && make BLAS=mkl

The libraries are the single-threaded ones (`-lblis`,
`mkl_sequential`) because there is one MPI rank per core. Run
`make clean` when you switch `BLAS` or `ARCH`.

## Quick tests on the login node

Small P and small n only. The login node has no InfiniBand, so
`run_local.sh` switches OpenMPI to shared memory.

The current `main` is a fixed-size correctness test: it builds the
11 x 12 `[A | b]`, prints it, factors it, and prints the result.

    ./factor_parallel [nb [P Q]]      # defaults: nb=2, P=2, Q=nranks/2

    ./run_local.sh 1 ./factor_parallel 1 1 1      # serial reference
    ./run_local.sh 4 ./factor_parallel            # nb=2, 2x2 grid
    ./run_local.sh 4 ./factor_parallel 3 4 1      # nb=3, 4x1 grid
    ./run_local.sh 6 ./factor_parallel 2 2 3      # nb=2, 2x3 grid
    make check PROG=factor_parallel NP=4 ARGS="2 2 2"

Pass P and Q together. With 1 rank or an odd number of ranks the default
P=2 doesn't fit, so give them explicitly. The `LU` block should be
identical for every nb, P and Q (save each output to a file and `diff`).
In the last row, U(11,11) and U(11,12) should be equal, because x = ones.

`run_local.sh` can also send stdin to the program:

    INPUT=8 ./run_local.sh 4 ./prog ...           # sends "8" on stdin

**stdin only reaches rank 0.** If the program reads the block size
with `scanf`, rank 0 has to read it and broadcast it. Otherwise the
other ranks get EOF. Command-line arguments reach every rank.

## Batch jobs (Slurm)

`main` now has two modes. With no args (or `nb [P Q]`) it runs the 11 x 12
print test. With `m n [P Q]` (first arg >= 8) it runs a timed factorization:
`nb` comes from stdin (default 1), P x Q defaults to `MPI_Dims_create`, and rank 0
prints one line

    RESULT m n nb P Q nranks time_s gflops gflops_per_rank rel_err

`rel_err` checks x = ones without gathering (column m vs row sums of U), so it
should be ~1e-14. GFLOPS uses (2/3) m^3.

Both scripts use account `26fa-cs598pa-eng`, partition
`eng-instruction`, and `--constraint=amd`, which selects the EPYC 7713
nodes (128 cores, Zen 3). Build first: the job runs whatever binary is
already in this directory.

### One run: `run.sbatch`

The allocation sets the rank count. Everything after the script name
is passed to the program unchanged.

    sbatch run.sbatch ./factor_parallel 2000 2001                   # 4 ranks
    sbatch --ntasks-per-node=64 run.sbatch ./factor_parallel 8000 8001
    INPUT=32 sbatch run.sbatch ./factor_parallel 4000 4001          # stdin
    sbatch --nodes=2 --ntasks-per-node=128 run.sbatch ./factor_parallel ...

Output goes to `lu-<jobid>.log`.

### Many runs in one job: `sweep.sbatch`

Write a cases file with one run per line (see `cases/example.txt`):

    # <nranks> <executable> [args...]     optional INPUT=<stdin> prefix
    INPUT=8  1 ./factor_parallel 4000 4001
    INPUT=8 16 ./factor_parallel 4000 4001
    INPUT=8 64 ./factor_parallel 4000 4001

then:

    sbatch sweep.sbatch cases/strong.txt          # 1 node, up to 128 ranks
    sbatch --nodes=2 sweep.sbatch cases/weak.txt  # up to 256 ranks

The sweep requests a whole node (`--exclusive`) so the timings aren't
affected by other jobs. A case that asks for more ranks than the
allocation has is skipped with a note in the log, and a failing case
doesn't stop the rest. Each case starts with a `=== np=... cmd=... ===`
line and ends with `=== exit <code> ===` in `lu-sweep-<jobid>.log`.
OpenMPI sometimes writes a NUL byte into the log, so search it with
`grep -a`.

How the program picks P and Q (arguments, `MPI_Dims_create`, ...) is
up to the code. The scripts only pass arguments through, so strong and
weak scaling are both just lists of lines in a cases file.

## Files

    layout.h         Layout struct, Aij macro, helper prototypes
    layout.c         block-cyclic helpers, test matrix, print_global
    factor_parallel.c  lu_unblocked and the test main
    Makefile         build rules described above
    run_local.sh     mpirun on the login node (shared-memory transport)
    run.sbatch       one Slurm run
    sweep.sbatch     a list of runs in one Slurm job
    cases/           example cases file
    mkl_shim/        cblas.h -> mkl_cblas.h, used only with BLAS=mkl

## Scaling runs and plots (Q1)

    python3 make_cases.py                      # cases/strong.txt weak_*.txt verify.txt
    sbatch sweep.sbatch cases/strong.txt
    sbatch sweep.sbatch cases/weak_1M.txt      # likewise weak_4M, weak_16M, verify
    python3 plot_results.py lu-sweep-*.log -o ../report/figs

`plot_results.py` writes `results.csv`, `strong.png`, `weak.png`, `gflops.png`,
`efficiency_footprint.png`, `efficiency_total.png`, and prints peak GFLOPS and,
per rank count, the smallest measured n^2/(PQ) from which every run reaches 80%
efficiency, plus the best efficiency seen. Strong scaling uses only runs from
`strong*` case files, weak scaling only `weak*` ones. On the footprint and
total-size plots efficiency = GFLOPS/rank / best 1-rank GFLOPS, so strong and
weak runs mean the same thing; the weak plot uses the 1-rank run with the same
n^2/(PQ).
