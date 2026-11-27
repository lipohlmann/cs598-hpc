#include "timers.h"

const char* const timer_names[TIMER_COUNT] = {
    "setup", "solve", "matvec", "kronecker", "gather_scatter", "allreduce"};

static double total[TIMER_COUNT];
static double started[TIMER_COUNT];
static long calls[TIMER_COUNT];

void timer_start(TimerId id) { started[id] = MPI_Wtime(); }

void timer_stop(TimerId id) {
  total[id] += MPI_Wtime() - started[id];
  calls[id]++;
}

void timer_reset(TimerId id) {
  total[id] = 0.0;
  calls[id] = 0;
}

void timers_summary(MPI_Comm comm, TimerSummary summary[TIMER_COUNT]) {
  int size;
  MPI_Comm_size(comm, &size);

  double mins[TIMER_COUNT], sums[TIMER_COUNT], maxs[TIMER_COUNT];
  long calls0[TIMER_COUNT];
  for (int i = 0; i < TIMER_COUNT; i++) calls0[i] = calls[i];

  MPI_Allreduce(total, mins, TIMER_COUNT, MPI_DOUBLE, MPI_MIN, comm);
  MPI_Allreduce(total, sums, TIMER_COUNT, MPI_DOUBLE, MPI_SUM, comm);
  MPI_Allreduce(total, maxs, TIMER_COUNT, MPI_DOUBLE, MPI_MAX, comm);
  MPI_Bcast(calls0, TIMER_COUNT, MPI_LONG, 0, comm);

  for (int i = 0; i < TIMER_COUNT; i++) {
    summary[i].min = mins[i];
    summary[i].avg = sums[i] / size;
    summary[i].max = maxs[i];
    summary[i].calls = calls0[i];
  }
}
