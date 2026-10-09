/* Rectangular Dirichlet Poisson: discrete exactness + O(h^2) convergence,
   with timings and the sampled solution exported as CSV.
   SPMD: each rank builds and checks only its own x-row slab.

   usage: test_poisson [Nx] [Ny] [Lx] [Ly] [allatonce|pairwise]
                       [num_reps] [max_sample_intervals] [output_dir]

     num_reps              timed solves of the continuous problem; the
                           fastest is reported               (default 5)
     max_sample_intervals  write the solution on at most this many
                           intervals per direction; 0 writes no
                           solution file                     (default 0)
     output_dir            directory for the CSV files (default results)

   Rank 0 writes its files per run, named by P, grid and transpose mode
   so that concurrent jobs never share a file:

     timing_<tag>.csv    one row: the fastest timed solve
     solution_<tag>.csv  one row per sampled interior grid point */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "msg.h"
#include "poisson.h"
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define TAG_SOLUTION 201

typedef struct {
  double total, fst, transpose, communication, gflops;
} solve_timing;

typedef enum { REFERENCE_RANDOM, REFERENCE_EXACT } reference_kind;

static double exact_u(double x, double y, double Lx, double Ly) {
  return sin(M_PI * x / Lx) * sin(2.0 * M_PI * y / Ly);
}

/* -Laplacian of the above */
static double rhs_f(double x, double y, double Lx, double Ly) {
  const double eigenvalue =
      M_PI * M_PI / (Lx * Lx) + 4.0 * M_PI * M_PI / (Ly * Ly);
  return eigenvalue * exact_u(x, y, Lx, Ly);
}

/* Pseudo-random value in [-1,1] for global grid point (row,col), the
   same on every rank, so a rank can evaluate any entry -- including its
   neighbours' rows -- without communication.  Zero outside the grid,
   which supplies the homogeneous Dirichlet data. */
static double rand_u(const poisson_plan* plan, int row, int col) {
  uint64_t hash;
  if (row < 0 || row >= plan->nx || col < 0 || col >= plan->ny) return 0.0;
  hash = (uint64_t)row * 0x9E3779B97F4A7C15ULL ^
         ((uint64_t)col + 999) * 0xC2B2AE3D27D4EB4FULL;
  hash ^= hash >> 30;
  hash *= 0xBF58476D1CE4E5B9ULL; /* splitmix64 finalizer */
  hash ^= hash >> 27;
  hash *= 0x94D049BB133111EBULL;
  hash ^= hash >> 31;
  return 2.0 * ((double)(hash >> 11) * (1.0 / 9007199254740992.0)) - 1.0;
}

/* Local slab of rhs = -Laplacian_h rand_u, global rows
   row_offset..row_offset+local_rows-1. */
static void rand_rhs(const poisson_plan* plan, int local_rows, int row_offset,
                     double* rhs) {
  const double inv_hx2 = 1.0 / (plan->hx * plan->hx);
  const double inv_hy2 = 1.0 / (plan->hy * plan->hy);
  int local_row, row, col;
  for (col = 0; col < plan->ny; ++col)
    for (local_row = 0; local_row < local_rows; ++local_row) {
      row = row_offset + local_row;
      const double center = rand_u(plan, row, col);
      rhs[local_row + col * local_rows] =
          inv_hx2 * (2.0 * center - rand_u(plan, row - 1, col) -
                     rand_u(plan, row + 1, col)) +
          inv_hy2 * (2.0 * center - rand_u(plan, row, col - 1) -
                     rand_u(plan, row, col + 1));
    }
}

/* Local slab of the continuous right-hand side. */
static void exact_rhs(const poisson_plan* plan, int local_rows, int row_offset,
                      double* rhs) {
  int local_row, col;
  for (col = 0; col < plan->ny; ++col) {
    const double y = (double)(col + 1) * plan->hy;
    for (local_row = 0; local_row < local_rows; ++local_row) {
      const double x = (double)(row_offset + local_row + 1) * plan->hx;
      rhs[local_row + col * local_rows] = rhs_f(x, y, plan->Lx, plan->Ly);
    }
  }
}

/* Global max|solution - reference|, the reference being rand_u or
   exact_u.  Collective. */
static double max_error(const poisson_plan* plan, int local_rows,
                        int row_offset, const double* solution,
                        reference_kind kind) {
  double error, local_max = 0.0, global_max;
  int local_row, col;
  for (col = 0; col < plan->ny; ++col) {
    const double y = (double)(col + 1) * plan->hy;
    for (local_row = 0; local_row < local_rows; ++local_row) {
      const double x = (double)(row_offset + local_row + 1) * plan->hx;
      const double reference =
          kind == REFERENCE_RANDOM
              ? rand_u(plan, row_offset + local_row, col)
              : exact_u(x, y, plan->Lx, plan->Ly);
      error = fabs(solution[local_row + col * local_rows] - reference);
      if (error > local_max) local_max = error;
    }
  }
  gmax_double(&local_max, &global_max, 1);
  return global_max;
}

/* Opens <output_dir>/<kind>_<run_tag>.csv for writing. */
static FILE* csv_open(const char* output_dir, const char* kind,
                      const char* run_tag) {
  char path[1024];
  FILE* file;
  if (strlen(output_dir) + strlen(kind) + strlen(run_tag) + 7 > sizeof path) {
    fprintf(stderr, "test_poisson: output path too long\n");
    return NULL;
  }
  sprintf(path, "%s/%s_%s.csv", output_dir, kind, run_tag);
  file = fopen(path, "w");
  if (!file)
    fprintf(stderr, "test_poisson: cannot write %s\n", path);
  else
    printf("wrote %s\n", path);
  return file;
}

/* Rank 0 only.  M is the largest number of x-rows on any rank (rank
   0's); the timings are those poisson_solve reports for one solve, each
   a max over ranks; error is max|U - u| for the continuous problem. */
static void write_timing_csv(const poisson_plan* plan,
                             const solve_timing* timing, double error,
                             const char* output_dir, const char* run_tag) {
  FILE* file = csv_open(output_dir, "timing", run_tag);
  if (!file) return;
  fprintf(file,
          "P,Nx,Ny,mode,M,t_total,t_fst,t_transpose,t_comm,gflops,error\n");
  fprintf(file, "%d,%d,%d,%s,%d,%.9e,%.9e,%.9e,%.9e,%.9e,%.9e\n", plan->P,
          plan->nx + 1, plan->ny + 1,
          plan->tmode == POISSON_TR_PAIRWISE ? "pairwise" : "allatonce",
          plan->mx[0], timing->total, timing->fst, timing->transpose,
          timing->communication, timing->gflops, error);
  fclose(file);
}

/* Grid lines k*stride, k = 1,2,..., are sampled, with the stride chosen
   so that at most max_sample_intervals intervals remain.  For a power
   of two num_intervals the sampled points are the same physical points
   for every num_intervals >= max_sample_intervals. */
static int sample_stride(int num_intervals, int max_sample_intervals) {
  return num_intervals > max_sample_intervals
             ? (num_intervals + max_sample_intervals - 1) /
                   max_sample_intervals
             : 1;
}

/* Number of sampled 0-based interior indices in [first, first+count). */
static int num_sampled(int first, int count, int stride) {
  return (first + count) / stride - first / stride;
}

/* Collective.  Every rank sends its sampled values to rank 0, which
   writes them with the exact solution in global (row,col) order. */
static void write_solution_csv(const poisson_plan* plan, int local_rows,
                               int row_offset, const double* solution,
                               int max_sample_intervals,
                               const char* output_dir, const char* run_tag) {
  const int my_rank = plan->rank;
  const int stride_x = sample_stride(plan->nx + 1, max_sample_intervals);
  const int stride_y = sample_stride(plan->ny + 1, max_sample_intervals);
  const int local_sampled_rows = num_sampled(row_offset, local_rows, stride_x);
  const int total_sampled_rows = plan->nx / stride_x;
  const int sampled_cols = plan->ny / stride_y;
  const size_t num_samples =
      (size_t)(my_rank == 0 ? total_sampled_rows : local_sampled_rows) *
      sampled_cols;
  double* samples =
      (double*)malloc((num_samples ? num_samples : 1) * sizeof(double));
  int local_row, sampled_row, sampled_col, rank, rows_received;
  FILE* file;

  if (!samples) {
    fprintf(stderr, "test_poisson: rank %d out of memory for solution\n",
            my_rank);
    return;
  }

  /* samples(sampled_col, sampled_row), sampled rows in increasing x */
  for (local_row = 0, sampled_row = 0; local_row < local_rows; ++local_row) {
    if ((row_offset + local_row + 1) % stride_x) continue;
    for (sampled_col = 0; sampled_col < sampled_cols; ++sampled_col) {
      const int col = (sampled_col + 1) * stride_y - 1;
      samples[sampled_col + (size_t)sampled_row * sampled_cols] =
          solution[local_row + (size_t)col * local_rows];
    }
    ++sampled_row;
  }

  if (my_rank == 0) {
    rows_received = local_sampled_rows;
    for (rank = 1; rank < plan->P; ++rank) {
      const int rank_sampled_rows =
          num_sampled(plan->xoff[rank], plan->mx[rank], stride_x);
      if (rank_sampled_rows)
        irecv(rank, samples + (size_t)rows_received * sampled_cols,
              rank_sampled_rows * sampled_cols * (int)sizeof(double),
              TAG_SOLUTION);
      rows_received += rank_sampled_rows;
    }
  } else if (local_sampled_rows) {
    isend(0, samples, local_sampled_rows * sampled_cols * (int)sizeof(double),
          TAG_SOLUTION);
  }
  msgwait();

  if (my_rank == 0 && (file = csv_open(output_dir, "solution", run_tag))) {
    fprintf(file, "i,j,x,y,u,u_exact,error\n");
    for (sampled_row = 0; sampled_row < total_sampled_rows; ++sampled_row) {
      const int grid_i = (sampled_row + 1) * stride_x; /* 1-based, x = i*hx */
      const double x = (double)grid_i * plan->hx;
      for (sampled_col = 0; sampled_col < sampled_cols; ++sampled_col) {
        const int grid_j = (sampled_col + 1) * stride_y;
        const double y = (double)grid_j * plan->hy;
        const double computed =
            samples[sampled_col + (size_t)sampled_row * sampled_cols];
        const double exact = exact_u(x, y, plan->Lx, plan->Ly);
        fprintf(file, "%d,%d,%.17g,%.17g,%.17g,%.17g,%.9e\n", grid_i, grid_j,
                x, y, computed, exact, computed - exact);
      }
    }
    fclose(file);
  }
  free(samples);
}

int main(int argc, char** argv) {
  int Nx = 128, Ny = 512, num_reps = 5, max_sample_intervals = 0, rep;
  double Lx = 2.0, Ly = 1.0, discrete_error, continuous_error;
  poisson_transpose_mode transpose_mode = POISSON_TR_ALLATONCE;
  const char* output_dir = "results";
  const char* mode_name;
  char run_tag[128];
  poisson_plan plan;
  solve_timing fastest = {0.0, 0.0, 0.0, 0.0, 0.0};
  double *rhs, *solution;

  msg_init(&argc, &argv);

  if (argc > 1) Nx = atoi(argv[1]);
  if (argc > 2) Ny = atoi(argv[2]);
  if (argc > 3) Lx = atof(argv[3]);
  if (argc > 4) Ly = atof(argv[4]);
  if (argc > 5 && !strcmp(argv[5], "pairwise"))
    transpose_mode = POISSON_TR_PAIRWISE;
  if (argc > 6) num_reps = atoi(argv[6]);
  if (argc > 7) max_sample_intervals = atoi(argv[7]);
  if (argc > 8) output_dir = argv[8];
  if (num_reps < 1) num_reps = 1;

  const int nx = Nx - 1, ny = Ny - 1; /* interior unknowns */
  const int my_rank = msg_rank();
  mode_name = transpose_mode == POISSON_TR_PAIRWISE ? "pairwise" : "allatonce";

  if (poisson_plan_init(&plan, nx, ny, Lx, Ly)) {
    msg_finalize();
    return 1;
  }
  plan.tmode = transpose_mode;
  const int local_rows = plan.mx[my_rank]; /* my x-rows */
  const int row_offset = plan.xoff[my_rank];

  rhs = (double*)malloc((size_t)local_rows * ny * sizeof(double));
  solution = (double*)malloc((size_t)local_rows * ny * sizeof(double));
  if (!rhs || !solution) {
    fprintf(stderr, "test_poisson: rank %d out of memory\n", my_rank);
    msg_finalize();
    return 2;
  }

  if (my_rank == 0) {
    printf("running on %d rank(s)\n", plan.P);
    printf("domain  [0,%g] x [0,%g]\n", Lx, Ly);
    printf("nx ny   = %d %d      (Nx Ny = %d %d)\n", nx, ny, Nx, Ny);
    printf("hx hy   = %.6e %.6e\n", plan.hx, plan.hy);
    printf("transpose = %s\n", mode_name);
  }

  /* ---- test 1: exact inversion of the DISCRETE operator ----------
     Also the warm-up solve; it is not timed. */
  rand_rhs(&plan, local_rows, row_offset, rhs);
  poisson_solve(&plan, rhs, solution);
  discrete_error =
      max_error(&plan, local_rows, row_offset, solution, REFERENCE_RANDOM);

  /* ---- test 2: continuous solution, expect O(h^2) ----------------
     Timed: keep the fastest of num_reps solves, all timers from that
     one. */
  exact_rhs(&plan, local_rows, row_offset, rhs);
  for (rep = 0; rep < num_reps; ++rep) {
    poisson_solve(&plan, rhs, solution);
    if (rep == 0 || plan.t_total < fastest.total) {
      fastest.total = plan.t_total;
      fastest.fst = plan.t_fst;
      fastest.transpose = plan.t_tr;
      fastest.communication = plan.t_comm;
      fastest.gflops = plan.gflops;
    }
  }
  continuous_error =
      max_error(&plan, local_rows, row_offset, solution, REFERENCE_EXACT);

  if (my_rank == 0) {
    printf("\ndiscrete solve  max|U - Uexact_h|      = %.6e   (expect "
           "~roundoff)\n",
           discrete_error);
    printf("continuous      max|U - u(x,y)|        = %.6e   (expect "
           "O(h^2))\n",
           continuous_error);
    printf("fastest of %d solves: total %.6e  fst %.6e  transpose %.6e  "
           "comm %.6e  GFLOPS %.3f\n\n",
           num_reps, fastest.total, fastest.fst, fastest.transpose,
           fastest.communication, fastest.gflops);
  }

  /* ---- CSV export ------------------------------------------------- */
  sprintf(run_tag, "P%04d_N%dx%d_%s", plan.P, Nx, Ny, mode_name);
  if (my_rank == 0) {
    mkdir(output_dir, 0777); /* may already exist; fopen reports real errors */
    write_timing_csv(&plan, &fastest, continuous_error, output_dir, run_tag);
  }
  if (max_sample_intervals > 0)
    write_solution_csv(&plan, local_rows, row_offset, solution,
                       max_sample_intervals, output_dir, run_tag);

  poisson_plan_free(&plan);
  free(rhs);
  free(solution);
  msg_finalize();
  return 0;
}
