# CS 598 HPC Midterm

Spectral element method (GLL quadrature) code using MPI. Rank 0 builds the
mesh, quadrature set, mass matrix and stiffness matrix, then broadcasts them
to all ranks. The elements are divided among the ranks with recursive
coordinate bisection (see [Mesh partitioning](#mesh-partitioning-rcb)).

## Requirements

- An MPI C compiler wrapper (`mpicc`) and launcher (`mpirun`), e.g. Open MPI
- `make`
- `latexmk` (only for building the presentation)

## Build

Clone with the gslib submodule (or fetch it in an existing clone):

```sh
git submodule update --init --recursive
```

Build the code (this builds gslib first, then the `midterm` executable):

```sh
make code
```

The default flags are `-O3 -Wall -Wextra -std=c11`. Pass extra architecture
flags with `ARCHFLAGS` (not `CFLAGS`, which would drop the gslib include path),
e.g. when building on a cluster compute node: `make code ARCHFLAGS=-march=native`.

Other targets:

```sh
make pdf        # build the presentation
make all        # code + pdf
make clean      # remove executable, objects, and LaTeX output
make distclean  # clean + remove the gslib build
```

## Run

Serial (one MPI rank):

```sh
./midterm
# or
mpirun -np 1 ./midterm
```

Parallel (e.g. 4 ranks):

```sh
mpirun -np 4 ./midterm
```

If you ask for more ranks than available cores, add `--oversubscribe`
(Open MPI):

```sh
mpirun --oversubscribe -np 8 ./midterm
```

`make run` runs the executable serially after building.

## Parameters

All parameters are command-line options (`./midterm --help`):

| Option | Meaning | Default |
| --- | --- | --- |
| `--nelem N` | elements per direction (sets `nx=ny=nz=N`) | 4 |
| `--nx`, `--ny`, `--nz` | elements in one direction | 4 |
| `--npts N` | GLL points per direction per element (>= 2) | 12 |
| `--length L`, `--lx`, `--ly`, `--lz` | domain length(s) | 1 |
| `--tol T` | CG relative residual tolerance | 1e-12 |
| `--max-iters N` | CG iteration cap | 1000 |
| `--csv FILE` | write one CSV row with results and timings | off |
| `--label STR` | label stored in the CSV | `run` |
| `--vtk PREFIX` | write `PREFIX_<rank>.vtu` + `PREFIX.pvtu` | off |

Example: `mpirun -np 8 ./midterm --nelem 16 --npts 6 --csv out.csv`.

Each run prints the CG iteration count, the L2 and max-norm errors, and the
time (maximum over ranks) spent in setup, the solve, the matvec, the Kronecker
products, the gslib gather-scatter and `MPI_Allreduce` (`src/timers.c`). The
timers are inclusive: the matvec contains the Kronecker products and the
gather-scatter, and the solve contains everything in CG. gslib also prints a
few lines about the communication method it chose during setup.

## Mesh partitioning (RCB)

Rank 0 builds the mesh, and each rank only owns (and does work on) a subset
of the elements. The subsets are chosen with **recursive coordinate
bisection** (`src/rcb.c`, `include/rcb.h`). RCB is a geometric partitioner: it
only uses the element centroids, not the element connectivity.

For `P` ranks, `rcb(elements, first_rank, P)` does:

1. If `P == 1`, all the elements belong to `first_rank`. Stop.
2. Compute the bounding box of the element centroids and pick its **longest
   axis** (x, y or z; ties go to the earlier axis). Always cutting the longest
   side keeps the pieces close to cubes, which minimizes the surface area, i.e.
   the number of nodes shared with other ranks that the gather-scatter has to
   exchange.
3. Sort the elements along that axis (ties are broken by element ID, so the
   result is deterministic).
4. Split the ranks into `P_left = P / 2` and `P_right = P - P_left`. The first
   `count * P_left / P` elements of the sorted list go to the left group, the
   rest to the right group. Splitting the elements in proportion to the ranks
   keeps the load balanced for any `P`, not only powers of two.
5. Recurse on the left group with ranks `[first_rank, first_rank + P_left)` and
   on the right group with the remaining ranks.

Example: 4x4x4 elements on 4 ranks. The first cut halves x (all axes tie), then
each half is halved along y, giving four 2x2x4 columns of 16 elements.

Rank 0 computes the partition (`rcb_partition`), groups the element IDs by
owner and scatters them (`scatter_partition`), so each rank receives only the
list of its own elements. The other ranks never get the global element or
vertex arrays: `bcast_mesh` only sends the structured-grid description (vertex
counts, spacings and the vertex coordinates along each axis, O(nx + ny + nz)
numbers), from which element IDs are mapped to grid positions arithmetically.
After partitioning, rank 0 frees its global arrays too. Each rank then builds
its gather-scatter handle, Dirichlet mask and inverse multiplicity from its own
elements (`setup_operator_local`).

Per-rank mesh memory and communication are therefore O(nx + ny + nz) plus
O(local elements). Rank 0 alone needs O(global mesh) memory, and only during
setup.

Properties and limits:

- Cost is O(E log E log P) for E elements. It only runs once at startup.
- Element counts differ by at most one in the cases tested (`P` = 1-8; the
  count balance is not proven for every `P`).
- If `P` is larger than the number of elements, some ranks get no elements.
- All elements are weighted equally, and the cut quality is based on geometry
  only (no graph/edge-cut optimization). The partition is static.
- On a 4x4x4 mesh with 8 ranks, RCB cuts 48 element faces versus 64 for
  contiguous blocks of element IDs, and 40 versus 80 on an 8x4x2 mesh.

## Verification (manufactured solution)

`main` solves \(-\nabla^2 u + u = f\) with homogeneous Dirichlet conditions
using unpreconditioned CG, and compares against a manufactured solution
(`src/mms.c`, `include/mms.h`):

- Exact solution: \(u_{ex}=\sin(4\pi x/L_x)\sin(2\pi y/L_y)\sin(3\pi z/L_z)\)
- Forcing: \(f=-\nabla^2 u_{ex}+u_{ex}=\left(\pi^2(16/L_x^2+4/L_y^2+9/L_z^2)+1\right)u_{ex}\)
- Element operator: the GLL mass matrices are diagonal, so each of the four
  Kronecker products in \(A_e = M_z\otimes M_y\otimes K_x + M_z\otimes K_y\otimes M_x +
  K_z\otimes M_y\otimes M_x + M_z\otimes M_y\otimes M_x\) needs only one dense 1D
  sweep; `SerialLocalOperator` (`src/kronecker.c`) does 3 n^4 multiply-adds
  per element with no temporaries (vs 12 n^4 for four generic products).
- Right-hand side: \(b = M f\), i.e. \(f\) at the GLL nodes times the lumped mass
  weights, summed over shared nodes (gather-scatter) and masked.

The program prints the CG iteration count and the global L2 and max-norm
errors. They do not depend on the number of MPI ranks. The CG tolerance and
iteration cap are set next to the other parameters in `main()`.

To use your own solution, edit `mms_u_exact()` and `mms_forcing()` in
`src/mms.c` (the forcing must equal \(-\nabla^2 u_{ex}+u_{ex}\)).
Because the operator enforces homogeneous Dirichlet conditions, \(u_{ex}\)
must be 0 on all six faces of the domain.

Example results (unit cube, 4x4x4 elements, varying GLL points per element).
The error drops by roughly a factor of 100 for every two extra points, i.e.
spectral convergence, until it reaches the CG tolerance / round-off (~1e-13):

| GLL points per element | CG iterations | L2 error |
| ---------------------- | ------------- | -------- |
| 4                      | 9             | 2.3e-3   |
| 6                      | 41            | 1.1e-5   |
| 8                      | 68            | 4.2e-8   |
| 10                     | 95            | 1.4e-10  |
| 12                     | 124           | 3.5e-13  |

Note that the errors are measured at the GLL nodes only. On very coarse meshes
every node can sit on a zero of \(u_{ex}\) (e.g. 2 elements with 3 points
put all x-nodes at multiples of 1/4, where \(\sin(4\pi x)=0\)); then
\(b=0\), \(u=0\) and the reported error is zero to round-off. Avoid such
meshes in convergence studies.

## Running the studies (Slurm)

The scripts in `scripts/` run the convergence, scaling and profiling studies.
Each run is an independent Slurm job that writes one CSV file, so jobs can run
in any order or in parallel.

1. Build on the cluster (load the MPI module first, e.g. `module load
   openmpi`): `make code`.
2. Preview what would be submitted: `DRY_RUN=1 ./scripts/run_all.sh`.
3. Submit, e.g.
   `ACCOUNT=myacct PARTITION=cpu TASKS_PER_NODE=128 SETUP_CMD="module load openmpi" ./scripts/run_all.sh`
   or run one study at a time:

| Script | Study | Main knobs (defaults) |
| --- | --- | --- |
| `run_p_convergence.sh` | p-convergence: fixed mesh, increase GLL points | `NELEM=4`, `NPTS="3 ... 16"`, `RANKS=4` |
| `run_h_convergence.sh` | h-convergence: refine equally in x, y, z at fixed order; finest run writes VTK | `NELEMS="3 6 12 24 48"`, `NPTS="3 5"`, `RANKS=32` |
| `run_strong_scaling.sh` | strong scaling: fixed problem, more ranks | `NELEM=64`, `NPTS=6`, `MAX_RANKS=256`, `MAX_ITERS=200`, `TOL=1e-10` |
| `run_weak_scaling.sh` | weak scaling: `BLOCK^3` elements per rank, constant h | `BLOCK=16`, `NPTS=6`, `MAX_RANKS=256`, `MAX_ITERS=200`, `TOL=1e-10` |

General settings (environment variables, see `scripts/common.sh`):
`ACCOUNT`, `PARTITION`, `TASKS_PER_NODE` (64), `TIME_LIMIT` (00:30:00),
`EXCLUSIVE` (1, whole nodes for clean timings), `SBATCH_EXTRA`, `SETUP_CMD`,
`LAUNCHER` (`srun --mpi=pmix --cpu-bind=cores`), `REPS` (5 repetitions of every timing
run), `RESULTS_DIR` (`./results`), `DRY_RUN`, and `SUBMIT=local` to run
directly with `mpirun` instead of Slurm (for small tests on a laptop).

The scaling studies cap CG at `MAX_ITERS` iterations so every run does the same
amount of work (they report "NOT CONVERGED"; that is expected). Weak scaling
uses power-of-two rank counts: the mesh doubles in x, then y, then z, and the
domain grows with it so that the element size and the work per rank stay
constant.

Output layout:

```
results/<study>/<label>.csv   one row per run (results + timings)
results/logs/                 program output of every job
results/vtk/finest.pvtu       finest h-convergence run (+ finest_<rank>.vtu)
```

4. When the jobs are done, make the figures and tables (needs pandas and
   matplotlib): `python scripts/analyze.py results`. It writes
   `results/figures/*.png` and `results/tables/*.{md,csv}`:
   - p- and h-convergence plots and tables, with observed h-rates,
   - strong scaling speedup and parallel efficiency `E = P0*T0 / (P*T(P))`,
   - weak scaling efficiency `E = T0 / T(P)`, both for the total solve time
     and per CG iteration (the unpreconditioned CG iteration count grows with
     the problem size, so the per-iteration number isolates parallel
     overhead),
   - the breakdown of the solve time into Kronecker products, gather-scatter,
     Allreduce, the rest of the matvec and the rest of CG, and the load
     imbalance (max / average over ranks).
5. Open `results/vtk/finest.pvtu` in ParaView. Point fields: `u`, `u_exact`,
   `error`; cell fields: `rank` (shows the RCB partition) and `element_id`.
   The files are ASCII and large for fine meshes (about 100 bytes per GLL
   node).

Known limits at scale: rank 0 builds the whole mesh and computes the RCB
partition serially, so rank 0's setup time and memory grow with the global
mesh size (setup is timed separately so this shows up in the results). The
other ranks only receive their own element list.
