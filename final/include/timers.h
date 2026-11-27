#pragma once

#include <mpi.h>

/**
 * @file timers.h
 * @brief Simple accumulating wall-clock timers (MPI_Wtime) for profiling.
 *
 * Each timer accumulates the time between timer_start() and timer_stop() and
 * counts how many times it was stopped. Timers are inclusive: TIMER_MATVEC
 * contains TIMER_KRONECKER and TIMER_GATHER_SCATTER, and TIMER_SOLVE contains
 * everything that happens inside the CG solve. A timer must not be started
 * again before it is stopped.
 */

typedef enum {
  TIMER_SETUP,           // mesh, partition, broadcasts, operator setup
  TIMER_SOLVE,           // whole conjugate gradient solve
  TIMER_MATVEC,          // whole operator application (apply_A)
  TIMER_KRONECKER,       // per-element Kronecker products inside apply_A
  TIMER_GATHER_SCATTER,  // gslib gs() call (direct stiffness summation)
  TIMER_ALLREDUCE,       // MPI_Allreduce inside global_dot
  TIMER_COUNT
} TimerId;

/** Short names, used as CSV column names. */
extern const char* const timer_names[TIMER_COUNT];

typedef struct {
  double min;  // minimum over ranks [s]
  double avg;  // average over ranks [s]
  double max;  // maximum over ranks [s]
  long calls;  // number of timer_stop() calls on rank 0
} TimerSummary;

/** @brief Start timer id. */
void timer_start(TimerId id);

/** @brief Stop timer id and add the elapsed time to its total. */
void timer_stop(TimerId id);

/** @brief Zero the accumulated time and call count of timer id. */
void timer_reset(TimerId id);

/**
 * @brief Reduce every timer over the ranks of comm. Collective; the result is
 * available on every rank.
 *
 * @param comm Communicator
 * @param summary Output array of TIMER_COUNT entries
 */
void timers_summary(MPI_Comm comm, TimerSummary summary[TIMER_COUNT]);
