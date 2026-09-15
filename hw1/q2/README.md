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
| `analysis/pingpong_stats.py` | the four answers, per class and socket — **stdlib only** |
| `analysis/plot_pingpong.py` | the figures — needs numpy + matplotlib |
| `data/` | result CSVs (committed) |

Each CSV row is one `(partner, message size)` pair:
`partner, partner_host, partner_cpu, partner_socket, class, nwds, bytes, nloop,
t_half_s, mb_per_s`, where `class` is `self` / `intra` / `inter` relative to
rank 0 and `partner_cpu` / `partner_socket` are where that rank was actually
running. The `#` header line records rank 0's own host, CPU and socket.

## Running on the cluster

From this directory on a login node:

```sh
./batch/submit_all.sh 2a     # P1 = 64 ranks on one node
squeue -u $USER
./batch/submit_all.sh 2b     # 1x128, 2x64, 2x96, 2x128
```

Every job asks for whole nodes (`--exclusive`) on the 128-CPU class
(`--exclude=ccc0398,ccc0399`). Only three such nodes exist, so the five jobs
run one or two at a time; the set takes roughly half an hour of wall clock
plus queue time. Outputs are `data/pp_P{64,128}_n1.csv` and
`data/pp_P{128,192,256}_n2.csv`, with one log per job in `logs/`. A config is
missing from `data/` if and only if its job failed.

**Check each log before trusting its data.** In order, it should contain:

1. `=== <date> P=<ranks> nodes=<n> ===` followed by `srun hostname` counts:
   the right number of ranks on the right number of nodes, all `ccc039[123]`.
2. The `lscpu` lines: the same model on every run, 2 sockets, 2 threads per
   core. (128 CPUs to Slurm is 64 physical cores — see the notes below.)
3. One `cpu-bind=... mask` line per rank from `srun --cpu-bind=verbose`. This
   is the placement the run actually got; the CSV's `partner_socket` column is
   the same information in usable form.
4. `pingpong: wrote <P x 500> rows to data/... in <t> s`. The largest run
   spends over eight minutes of a ten-minute limit measuring, so `<t>` is
   worth a glance.
5. The summary table from `pingpong_stats.py`, which the job runs on its own
   output. Every group in it should have a `/s0` or `/s1` suffix; a bare
   `intra` means the socket lookup failed on that node.

If a run has to be repeated, delete its CSV first: the analysis takes
whatever is in `data/`, and a stale file is indistinguishable from a fresh one
except by the `date=` in its header.

Locally (Open MPI, no Slurm), as a smoke test only:

```sh
make && ./run_local.sh
```

## Analysis

The numeric answers need no third-party packages, so they run anywhere,
including a login node:

```sh
python3 analysis/pingpong_stats.py data/pp_*.csv --per-partner
```

That prints latency, inverse bandwidth, `m_2` and the eager limit for every
**(class, socket)** group — `self/s0`, `intra/s0`, `intra/s1`, `inter/s0`,
`inter/s1` — as a median with the `[min, max]` spread across the ranks in the
group, and writes `analysis/out/summary_<tag>.{csv,tex}`. The `.tex` file is a
`booktabs` table that drops straight into `../report/report.tex`. The spread
is the check: within one group it should be tight; if it is not, the groups
are still mixing something.

Figures need the venv:

```sh
python3 -m venv .venv
.venv/bin/pip install -r analysis/requirements.txt
.venv/bin/python analysis/plot_pingpong.py data/pp_*.csv
```

This writes one log-log figure per configuration (all partner ranks as thin
lines, one median per group over them — solid for socket 0, dashed for socket
1) plus a cross-configuration comparison into `../report/figures/`.

## Notes on the measurement

- **Socket placement is the largest effect in the data, larger than rank
  count.** Slurm's default in-node task distribution is cyclic across sockets,
  so consecutive ranks alternate sockets; rank 0 lands on socket 0 (the CSV
  header's `socket0=` confirms it). In the first set of runs the two sockets'
  partners formed two clean populations: same-node partners on one socket ran
  15-50% slower than on the other depending on size, and across nodes one
  socket's partners were **3x** slower than the other's at 800 KB (2.3 vs 6.7
  GB/s — presumably the socket the NIC is not attached to).
  A plain per-class median sits between two populations and moves whenever the
  head count changes, which is exactly how the 2x96 run's inter-node
  "bandwidth" came out 8x worse than the 2x64 and 2x128 runs' with nothing
  physically different. Hence: the CSV records each partner's socket, the
  analysis groups on it, and the plots draw one median per group. Do not
  present a per-class number without its socket.
- **Jobs are `--exclusive`.** The first set of runs was not, and three of the
  five jobs were co-scheduled on shared nodes — the 1x64 job and half of the
  2x64 job sat on the two sockets of the same node at the same time. Which
  socket rank 0 landed on then depended on who got there first, and the
  same-node curves for the same host differed by 2x between runs. With whole
  nodes, rank 0 is on socket 0 every time and nothing else is running.
- **`eng-instruction` is heterogeneous, and its "cores" are hyperthreads.**
  `ccc0391-93` have 128 CPUs, `ccc0398-99` have 64. The CPU is a Xeon
  Platinum 8358 (32 cores per socket), so a 128-CPU node is 2 sockets x 32
  cores x 2 threads. Left to itself Slurm mixes the node classes, which broke
  the two large jobs outright (a 2-node allocation drawing one of each
  resolves fewer slots than `2 x ntasks-per-node`, and Open MPI refuses to
  launch); `batch/pingpong.slurm` pins every job to the 128-CPU class with
  `--exclude=ccc0398,ccc0399` — by name, because Slurm rejects the tidier
  `--mincpus=128` here. Two consequences worth stating in the report: the
  128-rank-per-node runs put two ranks on every physical core, and rank 0's
  sibling thread then hosts a spinning idle rank; and 64 and 128 ranks per
  node are therefore not the same experiment with more ranks.
- **Launch with `srun`, not `mpirun`.** `mpirun` on this cluster does not read
  the Slurm allocation — on a two-node job it sees only the local node and
  fails either as *Procs mapped: 64 ... PPR: 64:node* or as *binding more
  processes than cpus available in your allocation*, depending on `--map-by`.
  One-node jobs work fine either way, which hides it. `srun` gets the node
  list right, so it launches the ranks and `-np` / `--map-by` / `--bind-to`
  all go away; `--cpu-bind=verbose,cores` pins each rank and logs the mask.
- **The round trip is serialized.** The partner does `irecv; msgwait; isend;
  msgwait` — it cannot reply until the ping has landed. Posting both halves at
  once would let the legs overlap and would read roughly twice as fast.
- **Rank 0 pre-posts the return receive** before sending the ping, which is the
  normal way to avoid unexpected-message overhead on the reply. (The ping
  itself is usually an unexpected receive at the partner, whose `irecv` for
  the next iteration is posted only after its previous pong completes.)
- **The `p = 0` self case is one message per iteration**, not two, so its time
  is divided by `nloop` rather than `2 nloop`. Every curve is then the time
  per one-way message. (The first set of runs divided by `2 nloop` there and
  drew the self curve at half its true value.)
- **Separate ping and pong tags** — with one tag, the `p = 0` self case would
  match its own send.
- **Cached data**, as the problem statement asks: one send buffer and one
  distinct receive buffer, allocated once at the largest size and reused for
  every test.
- **Warm start at both the smallest and the largest size**, once globally
  before any measurement and again per partner. The first rendezvous-size
  message to a peer sets up its connection and registers memory; with an
  8-byte-only warm-up that cost landed inside the timed loop at the first size
  past the eager limit and showed up as a spike there.
- **Expect two steps in every curve**: a small one between 176 and 184 B (a
  short-message threshold) and the eager-to-rendezvous switch between 8088
  and 8224 B, i.e. the eager limit is 8 KB. `pingpong_stats.py` reports the
  latter as that bracket; the size grid is too coarse there to say more. The
  first step is what sets `m_2` for same-node partners (it crosses `2 t(1)`),
  whereas across nodes `m_2` is reached by the linear growth near 2 KB.
- **Inverse bandwidth is `t/m` at the largest sizes, not a slope fit.** There
  is a second protocol switch at 200–400 KB (it varies by group) where `t`
  roughly halves, so the curve is not linear over its top decade and a
  least-squares slope through it is meaningless — the first version of the
  script fit one and reported 90+ GB/s for a self-send. 808 KB has also not
  reached the asymptote; say so in the report.
- `num_ranks()` in the wrapper returns `MPI_Comm_size - 1`, i.e. it is really
  the largest valid rank. `pingpong.c` wraps this in `nranks()` / `max_rank()`
  so the quirk is stated in exactly one place.
- **Idle ranks spin.** Only ranks 0 and p are measured; the other P-2 sit in
  `MPI_Barrier`, which Open MPI polls aggressively. `--cpu-bind=cores` keeps
  each on its own CPU, but on a full node that CPU may be the sibling thread
  of the one under test (see above). If the intra-node curves look noisy,
  re-run with `OMPI_MCA_mpi_yield_when_idle=1` (commented in
  `batch/pingpong.slurm`) and compare — the difference is worth a sentence in
  the report.

## Knobs

`--msg-vol` (default `1e5`) sets `nloop = msg_vol/(nwds+2)`, clamped to
`[20,1000]`. It is the weak knob: the `nloop >= 20` floor dominates the cost at
large `m`. If a run threatens the 10-minute limit, cut `--nsizes` (which
truncates the largest messages) or `--max-rank` instead.
