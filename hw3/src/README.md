# hw3 / src: build and test harness

Build and run tooling for the parallel LU code. None of it refers to a
specific source file, so it keeps working as the code changes.

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

    ./run_local.sh 4 ./factor_parallel 300 301
    INPUT=8 ./run_local.sh 4 ./factor_parallel 300 301     # sends "8" on stdin
    make check PROG=factor_parallel NP=4 ARGS="300 301" INPUT=8

**stdin only reaches rank 0.** If the program reads the block size
with `scanf`, rank 0 has to read it and broadcast it. Otherwise the
other ranks get EOF. Command-line arguments reach every rank.

## Batch jobs (Slurm)

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

    Makefile         build rules described above
    run_local.sh     mpirun on the login node (shared-memory transport)
    run.sbatch       one Slurm run
    sweep.sbatch     a list of runs in one Slurm job
    cases/           example cases file
    mkl_shim/        cblas.h -> mkl_cblas.h, used only with BLAS=mkl
