# SPMD Dirichlet Poisson via batched FST + distributed transpose

Parallel (MPI) version of the serial solver in `../hw2_src`. The local
FST and tiled transpose kernels are unchanged; the solver distributes the
grid over P ranks and replaces the two serial transposes with
`isend`/`irecv`-based distributed transposes.

## Build and run

On the campus cluster (compute nodes, via Slurm):

    ./driver                      # module load, make clean, make, then submit
                                  # sweep, pingpong and fst_sweep jobs

    # single runs -- size comes from the sbatch options, args go to test_poisson
    sbatch --ntasks-per-node=16 poisson.sbatch 2048 2048 1 1
    sbatch --nodes=2 --ntasks-per-node=64 poisson.sbatch 2048 2048 1 1 pairwise

Quick checks on a login node (small P and N only):

    ./run_local.sh 4 ./test_poisson 128 512 2 1 pairwise

The login nodes have no InfiniBand, so the OpenMPI module's default UCX
transport fails there; `run_local.sh` switches to the `ob1` PML over
shared memory. Compute-node jobs don't need this.

Executables:

    test_poisson     [Nx] [Ny] [Lx] [Ly] [allatonce|pairwise]
                     [num_reps] [max_sample_intervals] [output_dir]   (MPI, any P)
    pingpong         [max_bytes] [output_dir] [label]           (MPI, P >= 2)
    test_block_fst   [m] [n] [nrep] [output_dir]                (serial)
    bench_transpose  [nrep]                                     (serial, local transpose)

Note `test_poisson` takes `Nx = nx+1`, the number of *intervals*.


## Allowed grid sizes

The radix-4 FST needs `L = 2*(n+1) = 4^k`, so

    n  = nx, ny  in {31, 127, 511, 2047, 8191, ...}
    N  = n+1     in {32, 128, 512, 2048, 8192, ...}

(The homework PDF suggests `n = 4^k - 1`; that gives 3, 15, 63, 255,
which this FST rejects.) nx and ny need not match. P must satisfy
`P <= min(nx, ny)`.


## Storage convention

Everything is **column-major with the first index unit-stride**, i.e.
Fortran ordering:

    A(i,j)  ==  A[i + j*m],     i = 0..m-1  (unit stride)
                                j = 0..n-1  (stride m)

`block_fst_apply()` on an m x n matrix **contracts over the second ("n")
index** and **vectorizes over the first ("m") index**; the m columns are
the batch, n is the transform length. `S_n` is the orthonormal DST-I,
symmetric and its own inverse.


## Decomposition

Both index ranges are split into contiguous blocks; the first `n mod P`
ranks get one extra entry. Since n is always odd, P never divides it
evenly. For power-of-two P the last rank gets one fewer row than the
others, so the busiest rank does no more work than a perfect split of N.

Rank p holds two slabs:

| slab | owns | local shape | layout | FST plan |
|---|---|---|---|---|
| A | x-rows `i in [xoff[p], xoff[p]+mx[p])`, all y | `mx[p] x ny` | `A[il + j*mx]` | `fy`: batch `mx`, length `ny` |
| B | y-cols `j in [yoff[p], yoff[p]+my[p])`, all x | `my[p] x nx` | `B[jl + i*my]` | `fx`: batch `my`, length `nx` |

`poisson_solve(p, F, U)` takes and returns **A-layout slabs**. With
P = 1 this is exactly the serial `nx x ny` layout.

Note the x-direction FST has batch size `my = ny/P`, so its vector
length shrinks as P grows; that is where single-core performance falls
off at large P.


## Method

    A  = F                                (mx x ny, local)
    A <- S_ny along y                     (local)
    B  = A^T                              (distributed transpose)
    B <- S_nx along x                     (local)
    B(jl,i) /= lam_x(i) + lam_y(yoff+jl)
    B <- S_nx along x                     (S is its own inverse)
    A  = B^T                              (distributed transpose)
    A <- S_ny along y
    U  = A

with the exact eigenvalues of the discrete operator,

    lam_x(i) = (4/hx^2) sin^2( pi*(i+1) / (2*(nx+1)) ).


## Distributed transpose

The index being redistributed is the slab's *second* index, so the block
rank p sends to rank r is a contiguous run of columns: no packing. Each
received block is transposed locally (`transpose_real`, tiled) straight
into a contiguous run of output columns. The rank's own block is a local
transpose with no message.

Two schedules, selected by `plan.tmode` (or the 5th `test_poisson` arg):

| mode | schedule |
|---|---|
| `POISSON_TR_ALLATONCE` (default) | post all P-1 `irecv`s, then all P-1 `isend`s (destinations staggered `p+1, p+2, ...`), do own block, one `msgwait` |
| `POISSON_TR_PAIRWISE` | P-1 rounds; round k sends to `(p+k)%P`, receives from `(p-k)%P`, waits, transposes that block |


## Timing and GFLOPS

After each solve the plan holds, as **max over ranks**:

| field | contents |
|---|---|
| `t_total` | whole solve (steps 1-7), after a barrier |
| `t_fst` | the four FST passes plus the eigenvalue divide |
| `t_tr` | both distributed transposes: messages **and** the local transposes of received blocks |
| `t_comm` | the part of `t_tr` spent in message calls (posting `isend`/`irecv`, `msgwait`) |
| `gflops` | aggregate rate over all P ranks |

GFLOPS uses the nominal count `~10 N log2 N` per length-N transform,
i.e. `20 nx ny (log2 Nx + log2 Ny)` for the whole solve. The code
actually runs a complex FFT of length `2(n+1)` per transform, so this is
a conventional figure for comparison, not the executed operation count.

Set `plan.verbose = 0` to silence the per-solve line rank 0 prints.


## Verification

`test_poisson` runs two checks. Each rank builds and checks only its own
slab; errors are max-reduced over ranks.

1. *Discrete exactness.* U is a pseudo-random field computed from a hash
   of the global index (i,j), so every rank can evaluate its neighbours'
   rows too and build its slab of `F = -Laplacian_h U` with no
   communication. Solving returns U to roundoff (~1e-13).

2. *Continuous convergence.* With `u = sin(pi x/Lx) sin(2 pi y/Ly)` and
   the continuous `-Delta u` as data, the error is O(h^2), independent
   of P. Measured (P = 4, square, L = 1):

       N =   32   2.734955e-03
       N =  128   1.706940e-04    ratio 16.02
       N =  512   1.066744e-05    ratio 16.00
       N = 2048   6.667111e-07    ratio 16.00

## CSV output

Every program writes into one output directory, `results` unless given
as its last argument.

| program | file | columns |
|---|---|---|
| `test_poisson` | `timing_<tag>.csv`, one row: the fastest of `num_reps` solves | `P,Nx,Ny,mode,M,t_total,t_fst,t_transpose,t_comm,gflops,error` |
| `test_poisson` with `max_sample_intervals > 0` | `solution_<tag>.csv` | `i,j,x,y,u,u_exact,error` |
| `pingpong` | `pingpong_<label>.csv` (`pingpong.csv` without a label) | `nbytes,nrep,t_oneway_min,t_oneway_avg` |
| `test_block_fst` | `fst.csv`, one row appended per timed run | `m,n,nrep,t_apply,gflops` |

`<tag>` is `P<P>_N<Nx>x<Ny>_<mode>`, so concurrent jobs never share a
file. `M` is the largest number of x-rows on any rank and `error` is the
max-norm error of the continuous problem.

Batch scripts that produce all of it (each takes the output directory as
its optional argument):

    sbatch sweep.sbatch        # test_poisson over N, P, both modes; merges timing.csv
    sbatch pingpong.sbatch     # pingpong_intranode.csv, pingpong_internode.csv
    sbatch fst_sweep.sbatch    # fst.csv over (m, n), single core


`poisson_residual_op()` (full-grid, serial) is kept in `poisson.c` but
no longer used by the test.
