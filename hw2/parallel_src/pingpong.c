/* Point-to-point ping-pong between ranks 0 and 1: one-way message time
   as a function of message size.  Any further ranks sit idle.

   usage: pingpong [max_bytes] [output_dir] [label]

     max_bytes   largest message; sizes are 8, 16, 32, ... bytes
                 (default 2^27, the largest transpose block at N=8192, P=2)
     output_dir  directory for the CSV file            (default results)
     label       names the placement of the two ranks, e.g. intranode or
                 internode; rank 0 writes <output_dir>/pingpong_<label>.csv
                 (default: <output_dir>/pingpong.csv)

   Each size is timed in NUM_TRIALS batches of num_reps round trips; a
   row holds the fastest and the average batch, as seconds per one-way
   message:

     nbytes,nrep,t_oneway_min,t_oneway_avg */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "msg.h"

#define TAG_PING 301
#define TAG_PONG 302
#define NUM_TRIALS 5

/* One round trip 0 -> 1 -> 0 as seen from rank my_rank (0 or 1). */
static void round_trip(int my_rank, char* message, int num_bytes) {
  if (my_rank == 0) {
    isend(1, message, num_bytes, TAG_PING);
    msgwait();
    irecv(1, message, num_bytes, TAG_PONG);
    msgwait();
  } else {
    irecv(0, message, num_bytes, TAG_PING);
    msgwait();
    isend(0, message, num_bytes, TAG_PONG);
    msgwait();
  }
}

/* Round trips per batch: many for short messages, few for long ones. */
static int reps_for_size(int num_bytes) {
  const int reps = (1 << 24) / num_bytes;
  return reps > 1000 ? 1000 : reps < 4 ? 4 : reps;
}

/* Rank 0 only: opens the CSV file and writes its header. */
static FILE* csv_open(const char* output_dir, const char* label) {
  char path[1024];
  FILE* file;
  if (strlen(output_dir) + strlen(label) + 16 > sizeof path) {
    fprintf(stderr, "pingpong: output path too long\n");
    return NULL;
  }
  if (label[0])
    sprintf(path, "%s/pingpong_%s.csv", output_dir, label);
  else
    sprintf(path, "%s/pingpong.csv", output_dir);
  mkdir(output_dir, 0777); /* may already exist; fopen reports real errors */
  file = fopen(path, "w");
  if (!file) {
    fprintf(stderr, "pingpong: cannot write %s\n", path);
    return NULL;
  }
  printf("writing %s\n", path);
  fprintf(file, "nbytes,nrep,t_oneway_min,t_oneway_avg\n");
  return file;
}

int main(int argc, char** argv) {
  int max_bytes = 1 << 27, num_bytes, rep, trial;
  const char* output_dir = "results";
  const char* label = "";
  char* message = NULL;
  FILE* csv_file = NULL;

  msg_init(&argc, &argv);
  const int my_rank = msg_rank();
  const int participating = my_rank < 2;

  if (argc > 1) max_bytes = atoi(argv[1]);
  if (argc > 2) output_dir = argv[2];
  if (argc > 3) label = argv[3];
  if (num_ranks() < 2 || max_bytes < 8) {
    if (my_rank == 0)
      fprintf(stderr, "usage: mpirun -np 2 pingpong [max_bytes>=8] "
                      "[output_dir] [label]\n");
    msg_finalize();
    return 1;
  }

  if (participating) {
    message = (char*)malloc((size_t)max_bytes);
    if (!message) {
      fprintf(stderr, "pingpong: rank %d out of memory\n", my_rank);
      msg_finalize();
      return 2;
    }
    memset(message, 0, (size_t)max_bytes); /* touch every page before timing */
  }
  if (my_rank == 0) {
    csv_file = csv_open(output_dir, label);
    printf("%12s %6s %14s %14s\n", "nbytes", "nrep", "t_oneway_min",
           "t_oneway_avg");
  }

  for (num_bytes = 8;
       participating && num_bytes > 0 && num_bytes <= max_bytes;
       num_bytes *= 2) {
    const int num_reps = reps_for_size(num_bytes);
    double fastest_time = 0.0, time_sum = 0.0;

    round_trip(my_rank, message, num_bytes); /* warm up */
    for (trial = 0; trial < NUM_TRIALS; ++trial) {
      const double start_time = msg_wtime();
      for (rep = 0; rep < num_reps; ++rep)
        round_trip(my_rank, message, num_bytes);
      const double one_way_time =
          (msg_wtime() - start_time) / (2.0 * num_reps);
      if (trial == 0 || one_way_time < fastest_time)
        fastest_time = one_way_time;
      time_sum += one_way_time;
    }
    if (my_rank == 0) {
      const double average_time = time_sum / NUM_TRIALS;
      printf("%12d %6d %14.6e %14.6e\n", num_bytes, num_reps, fastest_time,
             average_time);
      if (csv_file)
        fprintf(csv_file, "%d,%d,%.9e,%.9e\n", num_bytes, num_reps,
                fastest_time, average_time);
    }
  }

  if (csv_file) fclose(csv_file);
  free(message);
  msg_finalize();
  return 0;
}
