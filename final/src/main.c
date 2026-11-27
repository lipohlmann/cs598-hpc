#include <getopt.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "broadcast.h"
#include "cg.h"
#include "gather_scatter.h"
#include "gauss_lobatto_legendre.h"
#include "matrix.h"
#include "mesh.h"
#include "mms.h"
#include "operator.h"
#include "rcb.h"
#include "timers.h"
#include "vtk.h"

typedef struct {
  double Lx, Ly, Lz;        // domain lengths
  size_t nx, ny, nz;        // number of elements per direction
  int n_nodes_per_element;  // GLL points per direction (>= 2)
  double cg_rel_tol;        // CG stops at ||r|| / ||b|| < cg_rel_tol
  int cg_max_iters;         // CG iteration cap
  const char* csv_path;     // results file (NULL = none)
  const char* label;        // run label written to the CSV
  const char* vtk_prefix;   // VTK output prefix (NULL = none)
} Params;

static void print_usage(const char* prog) {
  fprintf(stderr,
          "Usage: %s [options]\n"
          "  --nelem N        elements per direction (sets nx=ny=nz=N)\n"
          "  --nx/--ny/--nz N elements in one direction\n"
          "  --npts N         GLL points per direction per element (>= 2)\n"
          "  --lx/--ly/--lz L domain length in one direction (> 0)\n"
          "  --length L       domain length in all directions\n"
          "  --tol T          CG relative residual tolerance (> 0)\n"
          "  --max-iters N    CG iteration cap (>= 1)\n"
          "  --csv FILE       write a one-row CSV with results and timings\n"
          "  --label STR      label stored in the CSV\n"
          "  --vtk PREFIX     write PREFIX_<rank>.vtu and PREFIX.pvtu\n"
          "  --help           show this message\n",
          prog);
}

static int parse_size(const char* text, size_t* out) {
  char* end;
  const long long value = strtoll(text, &end, 10);
  if (end == text || *end != '\0' || value < 1) return 1;
  *out = (size_t)value;
  return 0;
}

static int parse_int(const char* text, int min, int* out) {
  char* end;
  const long value = strtol(text, &end, 10);
  if (end == text || *end != '\0' || value < min || value > 1000000000L)
    return 1;
  *out = (int)value;
  return 0;
}

static int parse_positive_double(const char* text, double* out) {
  char* end;
  const double value = strtod(text, &end);
  if (end == text || *end != '\0' || !(value > 0.0)) return 1;
  *out = value;
  return 0;
}

// Returns 0 on success, 1 on a parse error, 2 if --help was requested.
static int parse_args(int argc, char* argv[], Params* p) {
  enum { OPT_NX = 256, OPT_NY, OPT_NZ, OPT_LX, OPT_LY, OPT_LZ, OPT_MAX_ITERS };
  static const struct option options[] = {
      {"nelem", required_argument, 0, 'e'},
      {"nx", required_argument, 0, OPT_NX},
      {"ny", required_argument, 0, OPT_NY},
      {"nz", required_argument, 0, OPT_NZ},
      {"npts", required_argument, 0, 'n'},
      {"lx", required_argument, 0, OPT_LX},
      {"ly", required_argument, 0, OPT_LY},
      {"lz", required_argument, 0, OPT_LZ},
      {"length", required_argument, 0, 'L'},
      {"tol", required_argument, 0, 't'},
      {"max-iters", required_argument, 0, OPT_MAX_ITERS},
      {"csv", required_argument, 0, 'c'},
      {"label", required_argument, 0, 'l'},
      {"vtk", required_argument, 0, 'v'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  opterr = 0;  // we print our own message
  int opt;
  while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
    int bad = 0;
    switch (opt) {
      case 'e':
        bad = parse_size(optarg, &p->nx);
        p->ny = p->nz = p->nx;
        break;
      case OPT_NX: bad = parse_size(optarg, &p->nx); break;
      case OPT_NY: bad = parse_size(optarg, &p->ny); break;
      case OPT_NZ: bad = parse_size(optarg, &p->nz); break;
      case 'n': bad = parse_int(optarg, 2, &p->n_nodes_per_element); break;
      case OPT_LX: bad = parse_positive_double(optarg, &p->Lx); break;
      case OPT_LY: bad = parse_positive_double(optarg, &p->Ly); break;
      case OPT_LZ: bad = parse_positive_double(optarg, &p->Lz); break;
      case 'L':
        bad = parse_positive_double(optarg, &p->Lx);
        p->Ly = p->Lz = p->Lx;
        break;
      case 't': bad = parse_positive_double(optarg, &p->cg_rel_tol); break;
      case OPT_MAX_ITERS: bad = parse_int(optarg, 1, &p->cg_max_iters); break;
      case 'c': p->csv_path = optarg; break;
      case 'l': p->label = optarg; break;
      case 'v': p->vtk_prefix = optarg; break;
      case 'h': return 2;
      default: return 1;  // unknown option or missing argument
    }
    if (bad) return 1;
  }
  return optind < argc ? 1 : 0;  // no stray positional arguments
}

static void write_csv(const char* path, const Params* p, int world_size,
                      int iters, double l2_error, double linf_error,
                      const TimerSummary timers[TIMER_COUNT]) {
  FILE* f = fopen(path, "w");
  if (!f) {
    fprintf(stderr, "error: cannot open %s for writing\n", path);
    return;
  }

  const unsigned long long m = (unsigned long long)p->n_nodes_per_element - 1;
  const unsigned long long dofs =
      (p->nx * m + 1) * (p->ny * m + 1) * (p->nz * m + 1);

  fprintf(f,
          "label,ranks,nx,ny,nz,npts,dofs,lx,ly,lz,tol,iters,l2_error,"
          "linf_error");
  for (int t = 0; t < TIMER_COUNT; t++)
    fprintf(f, ",t_%s_max,t_%s_avg,t_%s_min,n_%s", timer_names[t],
            timer_names[t], timer_names[t], timer_names[t]);
  fprintf(f, ",t_solve_per_iter\n");

  fprintf(f, "%s,%d,%zu,%zu,%zu,%d,%llu,%.17g,%.17g,%.17g,%.3e,%d,%.17g,%.17g",
          p->label, world_size, p->nx, p->ny, p->nz, p->n_nodes_per_element,
          dofs, p->Lx, p->Ly, p->Lz, p->cg_rel_tol, iters, l2_error,
          linf_error);
  for (int t = 0; t < TIMER_COUNT; t++)
    fprintf(f, ",%.9e,%.9e,%.9e,%ld", timers[t].max, timers[t].avg,
            timers[t].min, timers[t].calls);
  // time per CG iteration (one matvec + two dot products each)
  const long n_matvec = timers[TIMER_MATVEC].calls;
  fprintf(f, ",%.9e\n",
          n_matvec > 0 ? timers[TIMER_SOLVE].max / (double)n_matvec : 0.0);
  fclose(f);
}

int main(int argc, char* argv[]) {
  // defaults; override on the command line (see --help)
  Params params = {.Lx = 1.0,
                   .Ly = 1.0,
                   .Lz = 1.0,
                   .nx = 4,
                   .ny = 4,
                   .nz = 4,
                   .n_nodes_per_element = 12,
                   .cg_rel_tol = 1e-12,
                   .cg_max_iters = 1000,
                   .csv_path = NULL,
                   .label = "run",
                   .vtk_prefix = NULL};

  // initialize MPI
  MPI_Init(&argc, &argv);

  // get number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // every rank parses the same arguments, so they all agree on the outcome
  const int parse_status = parse_args(argc, argv, &params);
  if (parse_status != 0) {
    if (world_rank == 0) {
      if (parse_status == 1) fprintf(stderr, "error: invalid arguments\n");
      print_usage(argv[0]);
    }
    MPI_Finalize();
    return parse_status == 2 ? 0 : 1;
  }

  const double Lx = params.Lx, Ly = params.Ly, Lz = params.Lz;
  const size_t nx = params.nx, ny = params.ny, nz = params.nz;
  const int n_nodes_per_element = params.n_nodes_per_element;

  Mesh mesh = {0};
  QuadSet gll_quad = {0};
  Matrix M = {0};
  Matrix K = {0};
  Operator operator= { 0 };
  int* owner = NULL;  // rank that owns each element (rank 0 only)

  MPI_Barrier(MPI_COMM_WORLD);
  timer_start(TIMER_SETUP);

  // build everything on rank 0
  if (world_rank == 0) {
    create_mesh(&mesh, Lx, Ly, Lz, nx, ny, nz);
    gll_quad = GaussLobattoLegendre(n_nodes_per_element);
    owner = rcb_partition(&mesh, world_size);
    M = create_mass_matrix(&gll_quad);
    K = create_stiffness_matrix(&gll_quad);
    // map the reference element [-1,1] to an element of size dx, dy, dz
    operator.Kx = scale_by_constant(&K, 2.0 / mesh.dx);
    operator.Ky = scale_by_constant(&K, 2.0 / mesh.dy);
    operator.Kz = scale_by_constant(&K, 2.0 / mesh.dz);
    operator.Mx = scale_by_constant(&M, mesh.dx / 2.0);
    operator.My = scale_by_constant(&M, mesh.dy / 2.0);
    operator.Mz = scale_by_constant(&M, mesh.dz / 2.0);
  }
  operator.n = (size_t)n_nodes_per_element;

  // broadcast to all other ranks (collective)
  bcast_mesh(&mesh, 0, MPI_COMM_WORLD);

  // send every rank only the IDs of its own elements
  size_t* local_element_ids = NULL;
  size_t n_local_elements = 0;
  scatter_partition(owner, nx * ny * nz, &local_element_ids, &n_local_elements,
                    0, MPI_COMM_WORLD);

  // the global vertex/element arrays and the owner map were only needed to
  // partition; drop them so rank 0 keeps the same lightweight mesh as others
  free(owner);
  owner = NULL;
  free(mesh.node_ids);
  mesh.node_ids = NULL;
  free(mesh.elements);
  mesh.elements = NULL;

  bcast_quad_set(&gll_quad, n_nodes_per_element, 0, MPI_COMM_WORLD);
  bcast_matrix(&M, 0, MPI_COMM_WORLD);
  bcast_matrix(&K, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Kx, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Ky, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Kz, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Mx, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.My, 0, MPI_COMM_WORLD);
  bcast_matrix(&operator.Mz, 0, MPI_COMM_WORLD);

  // per-rank parts: element partition, mask, gather-scatter, inv_mult
  setup_operator_local(&operator, &mesh, local_element_ids, n_local_elements,
                       MPI_COMM_WORLD);
  timer_stop(TIMER_SETUP);

  // manufactured solution: right-hand side, CG solve, error against exact
  MmsParams mms_params = {Lx, Ly, Lz};
  Vector b = create_rhs(&operator, &mesh, &gll_quad, mms_forcing, &mms_params);
  Vector u_exact =
      evaluate_field(&operator, &mesh, &gll_quad, mms_u_exact, &mms_params);
  Vector u = create_vector(b.size);  // zero initial guess

  // only time the solve: clear what setup and the RHS accumulated
  for (int t = TIMER_SOLVE; t < TIMER_COUNT; t++) timer_reset((TimerId)t);
  MPI_Barrier(MPI_COMM_WORLD);
  timer_start(TIMER_SOLVE);
  int iters = conjugate_gradient(&u, &b, params.cg_rel_tol,
                                 params.cg_max_iters, &operator);
  timer_stop(TIMER_SOLVE);

  double l2_error, linf_error;
  mms_error_norms(&operator, &u, &u_exact, &l2_error, &linf_error);

  TimerSummary timers[TIMER_COUNT];
  timers_summary(MPI_COMM_WORLD, timers);

  if (world_rank == 0) {
    printf("ranks %d | CG iterations %d%s | L2 error %.6e | Linf error %.6e\n",
           world_size, iters, iters < 0 ? " (NOT CONVERGED)" : "", l2_error,
           linf_error);
    printf(
        "time [s, max over ranks]: setup %.4e | solve %.4e | matvec %.4e |"
        " kronecker %.4e | gather-scatter %.4e | allreduce %.4e\n",
        timers[TIMER_SETUP].max, timers[TIMER_SOLVE].max,
        timers[TIMER_MATVEC].max, timers[TIMER_KRONECKER].max,
        timers[TIMER_GATHER_SCATTER].max, timers[TIMER_ALLREDUCE].max);
    if (params.csv_path)
      write_csv(params.csv_path, &params, world_size, iters, l2_error,
                linf_error, timers);
  }

  if (params.vtk_prefix) {
    if (write_vtu_parallel(params.vtk_prefix, &operator, &mesh, &gll_quad, &u,
                           &u_exact, MPI_COMM_WORLD) != 0) {
      if (world_rank == 0)
        fprintf(stderr, "error: failed to write VTK output %s\n",
                params.vtk_prefix);
    } else if (world_rank == 0) {
      printf("wrote %s.pvtu\n", params.vtk_prefix);
    }
  }

  free_vector(&b);
  free_vector(&u_exact);
  free_vector(&u);

  free_operator(&operator);
  free_mesh(&mesh);
  free_quad_set(&gll_quad);
  free_matrix(&M);
  free_matrix(&K);

  MPI_Finalize();

  return 0;
}
