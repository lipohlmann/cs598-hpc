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

Domain size, number of elements per direction, and GLL points per element are
set at the top of `main()` in `src/main.c`; edit them and rebuild.

## Mesh partitioning (RCB)

Every rank receives the full mesh, but each rank only owns (and does work on)
a subset of the elements. The subsets are chosen with **recursive coordinate
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

Rank 0 computes the partition (`rcb_partition`) and broadcasts the owner of
each element (`bcast_partition`) so that all ranks use exactly the same
partition. Each rank then builds its gather-scatter handle, Dirichlet mask and
inverse multiplicity from its own elements (`setup_operator_local`).

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
