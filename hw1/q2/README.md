# HW1 Q2 — MPI ping-pong test

Measures the 1/2-round-trip message time between rank `q = 0` and every rank
`p = 0..M`, over the 500-point pseudo-geometric sweep of message sizes given in
the problem statement (`m = 1..64` exactly, then geometric out to 101,086 words
≈ 808 KB).

Written against the course's "MPI-free" wrapper (`include/msg.h`,
`src/msg_mpi.c`, copied verbatim from `../../scripts/`). `src/pingpong.c` never
includes `mpi.h`.

## Layout

| Path | What it is |
|---|---|
| `src/pingpong.c` | the benchmark: timing kernel + driver |
| `src/msg_mpi.c`, `include/msg.h` | course-provided wrappers, unmodified |
| `batch/pingpong.slurm` | one generic Slurm job, parameterized from the command line |
| `batch/submit_all.sh` | submits all five 2a/2b configurations |
| `run_local.sh` | short shared-memory smoke test, no Slurm |
| `analysis/pingpong_stats.py` | the four answers — **stdlib only** |
| `analysis/plot_pingpong.py` | the figures — needs numpy + matplotlib |
| `data/` | result CSVs (committed) |

## Running

On the cluster, from this directory:

```sh
./batch/submit_all.sh 2a     # the P1 = 64-rank baseline first
squeue -u $USER
./batch/submit_all.sh 2b     # then 1x128, 2x64, 2x96, 2x128
```

All five must run on the same node class or the comparison is meaningless --
see the note on the heterogeneous partition below. The five output files are
`data/pp_P{64,128}_n1.csv` and `data/pp_P{128,192,256}_n2.csv`; a config is
missing from `data/` if and only if its job failed.

Locally (Open MPI, no Slurm):

```sh
make && ./run_local.sh
```

## Analysis

The numeric answers need no third-party packages, so they run anywhere,
including a login node:

```sh
python3 analysis/pingpong_stats.py data/pp_P64_n1.csv --per-partner
```

That prints latency, inverse bandwidth, `m_2` and the eager limit per rank
class, and writes `analysis/out/summary_<tag>.{csv,tex}`. The `.tex` file is a
`booktabs` table that drops straight into `../report/report.tex`.

Figures need the venv:

```sh
python3 -m venv .venv
.venv/bin/pip install -r analysis/requirements.txt
.venv/bin/python analysis/plot_pingpong.py data/pp_*.csv
```

This writes one log-log figure per configuration plus a cross-configuration
comparison into `../report/figures/`.

## Notes on the measurement

- **`eng-instruction` is heterogeneous.** `ccc0391-93` have 128 cores,
  `ccc0398-99` have 64. Left to itself Slurm mixes them, which broke the two
  large jobs outright (a 2-node allocation drawing one of each resolves fewer
  slots than `2 x ntasks-per-node`, and Open MPI refuses to launch) and, more
  quietly, put the 1x64 baseline on a 64-core node and the 1x128 run on a
  128-core one -- so their latency difference was mostly a CPU difference, not
  a rank-count effect. `batch/pingpong.slurm` pins every job to the 128-core
  class with `--exclude=ccc0398,ccc0399` — by name, because Slurm rejects the
  tidier `--mincpus=128` here. Do not compare curves across node classes.
- **Launch with `srun`, not `mpirun`.** `mpirun` on this cluster does not read
  the Slurm allocation — on a two-node job it sees only the local node and
  fails either as *Procs mapped: 64 ... PPR: 64:node* or as *binding more
  processes than cpus available in your allocation*, depending on `--map-by`.
  One-node jobs work fine either way, which hides it. `srun` gets the node
  list right, so it launches the ranks and `-np` / `--map-by` / `--bind-to`
  all go away.
- **The round trip is serialized.** The partner does `irecv; msgwait; isend;
  msgwait` — it cannot reply until the ping has landed. Posting both halves at
  once would let the legs overlap and would read roughly twice as fast.
- **Rank 0 pre-posts the return receive** before sending the ping, which is the
  normal way to avoid unexpected-message overhead on the reply.
- **Separate ping and pong tags** — with one tag, the `p = 0` self case would
  match its own send.
- **Cached data**, as the problem statement asks: one send buffer and one
  distinct receive buffer, allocated once at the largest size and reused for
  every test.
- **Warm start** twice: once globally before any measurement, and again per
  partner before that partner's sweep.
- `num_ranks()` in the wrapper returns `MPI_Comm_size - 1`, i.e. it is really
  the largest valid rank. `pingpong.c` wraps this in `nranks()` / `max_rank()`
  so the quirk is stated in exactly one place.
- **Idle ranks spin.** Only ranks 0 and p are measured; the other P-2 sit in
  `MPI_Barrier`, which Open MPI polls aggressively. `--bind-to core` keeps them
  off the two cores under test. If the intra-node curves look noisy, re-run
  with `OMPI_MCA_mpi_yield_when_idle=1` (commented in `batch/pingpong.slurm`)
  and compare — the difference is worth a sentence in the report.

## Knobs

`--msg-vol` (default `1e5`) sets `nloop = msg_vol/(nwds+2)`, clamped to
`[20,1000]`. It is the weak knob: the `nloop >= 20` floor dominates the cost at
large `m`. If a run threatens the 10-minute limit, cut `--nsizes` (which
truncates the largest messages) or `--max-rank` instead.
