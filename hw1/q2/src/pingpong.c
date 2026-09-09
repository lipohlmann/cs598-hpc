/*
 * pingpong.c -- CS598 HPC, Homework 1, Q2.
 *
 * Point-to-point ping-pong test.  Rank q=0 exchanges messages with every rank
 * p = 0,1,...,M and records the 1/2-round-trip time as a function of message
 * size m (in 64-bit words).
 *
 * Written entirely against the "MPI-free" wrapper in msg.h; this file does not
 * include mpi.h.
 *
 * Message sizes follow the pseudo-geometric progression given in the problem
 * statement, which yields 500 strictly increasing sizes covering m = 1..64
 * exactly and running out to m = 101086 words (about 808 KB).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "msg.h"

#define TAG_PING 1
#define TAG_PONG 2
#define TAG_HOST 3

#define HOSTLEN 64
#define NSIZES_MAX 500

/* Defaults; all overridable from the command line. */
#define DEF_MSG_VOL 1.0e5
#define DEF_NSIZES NSIZES_MAX
#define DEF_MAXRANK 512 /* problem statement's suggested cap on M */
#define WARMUP_LOOPS 5

/*
 * num_ranks() in msg_mpi.c returns MPI_Comm_size-1, i.e. it is really the
 * largest valid rank.  Keep that fact in exactly one place.
 */
static int max_rank(void) { return num_ranks(); }
static int nranks(void) { return num_ranks() + 1; }

/* ------------------------------------------------------------------ */
/* The timing kernel.                                                  */
/*                                                                     */
/* Both participants call this; the branch picks the role.  The two    */
/* separate msgwait() calls on the partner side are what make this a   */
/* serialized round trip: the pong cannot leave until the ping has     */
/* landed.  Posting both halves at once instead would let the two legs */
/* overlap and would read roughly twice as fast.                       */
/*                                                                     */
/* Rank 0 returns the half-round-trip time in seconds; the partner     */
/* returns -1.                                                         */
/* ------------------------------------------------------------------ */
static double ping_pong(int me, int p, const double *sbuf, double *rbuf,
                        int nbytes, int nloop) {
  int i;
  double t0, t1;

  if (me == 0 && p == 0) {
    /* Self test: one send and one matching receive on the same rank. */
    t0 = msg_wtime();
    for (i = 0; i < nloop; ++i) {
      irecv(0, rbuf, nbytes, TAG_PING);
      isend(0, sbuf, nbytes, TAG_PING);
      msgwait();
    }
    t1 = msg_wtime();
    return (t1 - t0) / (2.0 * nloop);
  }

  if (me == 0) {
    t0 = msg_wtime();
    for (i = 0; i < nloop; ++i) {
      irecv(p, rbuf, nbytes, TAG_PONG); /* pre-post the return leg */
      isend(p, sbuf, nbytes, TAG_PING);
      msgwait(); /* completes only once p has replied */
    }
    t1 = msg_wtime();
    return (t1 - t0) / (2.0 * nloop);
  }

  for (i = 0; i < nloop; ++i) { /* me == p */
    irecv(0, rbuf, nbytes, TAG_PING);
    msgwait();
    isend(0, sbuf, nbytes, TAG_PONG);
    msgwait();
  }
  return -1.0;
}

/* ------------------------------------------------------------------ */
/* Payload check.  Rank 0 sends a known pattern; the partner echoes    */
/* back exactly the bytes it received, so a mismatch means the ping    */
/* itself was corrupted or mismatched.  This deliberately does not go  */
/* through ping_pong(), which always sends the partner's own sbuf and  */
/* so could not tell a correct exchange from a dropped one.  Runs once */
/* per partner, outside every timed region.                            */
/* ------------------------------------------------------------------ */
#define VERIFY_WORD(p, i) (1000.0 * ((p) + 1) + (i))

static int verify_exchange(int me, int p, double *sbuf, double *rbuf,
                           int nwds) {
  const int nbytes = 8 * nwds;
  int i;

  memset(rbuf, 0, (size_t)nbytes);

  if (p == 0) { /* self */
    for (i = 0; i < nwds; ++i) sbuf[i] = VERIFY_WORD(p, i);
    irecv(0, rbuf, nbytes, TAG_PING);
    isend(0, sbuf, nbytes, TAG_PING);
    msgwait();
  } else if (me == 0) {
    for (i = 0; i < nwds; ++i) sbuf[i] = VERIFY_WORD(p, i);
    irecv(p, rbuf, nbytes, TAG_PONG);
    isend(p, sbuf, nbytes, TAG_PING);
    msgwait();
  } else {
    irecv(0, rbuf, nbytes, TAG_PING);
    msgwait();
    memcpy(sbuf, rbuf, (size_t)nbytes); /* echo what actually arrived */
    isend(0, sbuf, nbytes, TAG_PONG);
    msgwait();
  }

  if (me != 0) return 0;

  for (i = 0; i < nwds; ++i) {
    if (rbuf[i] != VERIFY_WORD(p, i)) {
      fprintf(stderr, "verify: partner %d word %d: got %g want %g\n", p, i,
              rbuf[i], VERIFY_WORD(p, i));
      return 1;
    }
  }
  return 0;
}

/* ------------------------------------------------------------------ */
static void usage(const char *prog) {
  fprintf(stderr,
          "usage: %s [options]\n"
          "  --msg-vol V    message volume controlling nloop (default %g)\n"
          "  --max-rank M   highest partner rank to test (default min(P-1,%d))\n"
          "  --nsizes  J    number of message sizes, <= %d (default %d)\n"
          "  --nodes   N    node count, recorded in the output header only\n"
          "  -o FILE        output CSV (default stdout)\n"
          "  --verify       check payloads round-trip intact\n",
          prog, DEF_MSG_VOL, DEF_MAXRANK, NSIZES_MAX, DEF_NSIZES);
}

int main(int argc, char *argv[]) {
  double msg_vol = DEF_MSG_VOL;
  int nsizes = DEF_NSIZES;
  int M = -1;
  int nodes = 1;
  int verify = 0;
  const char *outfile = NULL;

  int nwds[NSIZES_MAX], nloop_tab[NSIZES_MAX];
  char myhost[HOSTLEN], host0[HOSTLEN];
  char *hosts = NULL;
  double *thalf = NULL, *sbuf = NULL, *rbuf = NULL;
  int me, P, p, j, i, maxwds, warm_partner, nbad = 0;
  double t_start;

  msg_init(&argc, &argv);
  me = msg_rank();
  P = nranks();

  for (i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--msg-vol") && i + 1 < argc) {
      msg_vol = atof(argv[++i]);
    } else if (!strcmp(argv[i], "--max-rank") && i + 1 < argc) {
      M = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "--nsizes") && i + 1 < argc) {
      nsizes = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "--nodes") && i + 1 < argc) {
      nodes = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-o") && i + 1 < argc) {
      outfile = argv[++i];
    } else if (!strcmp(argv[i], "--verify")) {
      verify = 1;
    } else {
      if (me == 0) {
        fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
        usage(argv[0]);
      }
      msg_finalize();
      return 1;
    }
  }

  if (nsizes < 1 || nsizes > NSIZES_MAX) {
    if (me == 0) fprintf(stderr, "--nsizes must be in [1,%d]\n", NSIZES_MAX);
    msg_finalize();
    return 1;
  }
  if (msg_vol <= 0.0) {
    if (me == 0) fprintf(stderr, "--msg-vol must be positive\n");
    msg_finalize();
    return 1;
  }

  if (M < 0) M = max_rank();
  if (M > DEF_MAXRANK) M = DEF_MAXRANK;
  if (M > max_rank()) M = max_rank();

  /* Message-size table, exactly as specified in the problem statement. */
  {
    int n = 0;
    for (j = 0; j < nsizes; ++j) {
      double nl;
      n = (int)((n + 1) * 1.016);
      nwds[j] = n;
      nl = msg_vol / ((double)n + 2.0);
      if (nl > 1000.0) nl = 1000.0;
      if (nl < 20.0) nl = 20.0;
      nloop_tab[j] = (int)nl;
    }
  }
  maxwds = nwds[nsizes - 1];

  /*
   * One send buffer and one (distinct) receive buffer, reused for every test
   * so that the data stays cached -- this is the measurement the problem
   * statement asks for.
   */
  sbuf = malloc((size_t)maxwds * sizeof(double));
  rbuf = malloc((size_t)maxwds * sizeof(double));
  if (!sbuf || !rbuf) {
    fprintf(stderr, "rank %d: out of memory for %d-word buffers\n", me, maxwds);
    msg_finalize();
    return 1;
  }
  for (i = 0; i < maxwds; ++i) sbuf[i] = (double)i;
  memset(rbuf, 0, (size_t)maxwds * sizeof(double));

  if (gethostname(myhost, sizeof(myhost)) != 0) strcpy(myhost, "unknown");
  myhost[HOSTLEN - 1] = '\0';

  if (me == 0) {
    thalf = malloc((size_t)(M + 1) * nsizes * sizeof(double));
    hosts = malloc((size_t)(M + 1) * HOSTLEN);
    if (!thalf || !hosts) {
      fprintf(stderr, "rank 0: out of memory for results\n");
      msg_finalize();
      return 1;
    }
    memcpy(host0, myhost, HOSTLEN);
    fprintf(stderr, "pingpong: P=%d nodes=%d M=%d nsizes=%d msg_vol=%g\n", P,
            nodes, M, nsizes, msg_vol);
    fprintf(stderr, "pingpong: sizes %d..%d words (%d..%d bytes)\n", nwds[0],
            maxwds, 8 * nwds[0], 8 * maxwds);
  }

  /* Warm start the timer and the connection path before any measurement. */
  warm_partner = (M >= 1) ? 1 : 0;
  msg_barrier();
  if (me == 0 || me == warm_partner)
    ping_pong(me, warm_partner, sbuf, rbuf, 8 * nwds[0], WARMUP_LOOPS);

  t_start = msg_wtime();

  for (p = 0; p <= M; ++p) {
    msg_barrier(); /* every rank reaches this, including idle ones */
    if (me != 0 && me != p) continue;

    /*
     * Exchange hostnames so rank 0 can label each partner intra- or
     * inter-node later.  msg.h exposes no gather, so this rides on the
     * point-to-point calls we already have.
     */
    if (p == 0) {
      memcpy(&hosts[0], myhost, HOSTLEN);
    } else if (me == 0) {
      irecv(p, &hosts[(size_t)p * HOSTLEN], HOSTLEN, TAG_HOST);
      msgwait();
    } else {
      isend(0, myhost, HOSTLEN, TAG_HOST);
      msgwait();
    }

    if (verify) {
      nbad += verify_exchange(me, p, sbuf, rbuf, nwds[0] < 8 ? 8 : nwds[0]);
      for (i = 0; i < maxwds; ++i) sbuf[i] = (double)i; /* restore payload */
    }

    /* Per-partner warm start: first touch of this pair's connection. */
    ping_pong(me, p, sbuf, rbuf, 8 * nwds[0], WARMUP_LOOPS);

    for (j = 0; j < nsizes; ++j) {
      double t = ping_pong(me, p, sbuf, rbuf, 8 * nwds[j], nloop_tab[j]);
      if (me == 0) thalf[(size_t)p * nsizes + j] = t;
    }

    if (me == 0 && (p % 16 == 0 || p == M))
      fprintf(stderr, "  partner %4d/%d  (%.1f s elapsed)\n", p, M,
              msg_wtime() - t_start);
  }

  msg_barrier();

  /* All output happens here, well outside every timed region. */
  if (me == 0) {
    FILE *fp = outfile ? fopen(outfile, "w") : stdout;
    time_t now = time(NULL);
    char stamp[32];

    if (!fp) {
      fprintf(stderr, "cannot open %s for writing\n", outfile);
      msg_finalize();
      return 1;
    }
    strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%S", localtime(&now));

    fprintf(fp, "# P=%d nodes=%d M=%d msg_vol=%g nsizes=%d host0=%s date=%s\n",
            P, nodes, M, msg_vol, nsizes, host0, stamp);
    fprintf(fp, "partner,partner_host,class,nwds,bytes,nloop,t_half_s,mb_per_s\n");

    for (p = 0; p <= M; ++p) {
      const char *host = &hosts[(size_t)p * HOSTLEN];
      const char *cls =
          (p == 0) ? "self" : (strcmp(host, host0) == 0 ? "intra" : "inter");
      for (j = 0; j < nsizes; ++j) {
        double t = thalf[(size_t)p * nsizes + j];
        double mbs = (t > 0.0) ? (8.0 * nwds[j]) / t / 1.0e6 : 0.0;
        fprintf(fp, "%d,%s,%s,%d,%d,%d,%.9e,%.4f\n", p, host, cls, nwds[j],
                8 * nwds[j], nloop_tab[j], t, mbs);
      }
    }

    if (fp != stdout) fclose(fp);
    fprintf(stderr, "pingpong: wrote %d rows to %s in %.1f s\n",
            (M + 1) * nsizes, outfile ? outfile : "stdout",
            msg_wtime() - t_start);
    if (verify)
      fprintf(stderr, "pingpong: verify %s (%d mismatched partners)\n",
              nbad ? "FAILED" : "passed", nbad);
  }

  free(sbuf);
  free(rbuf);
  free(thalf);
  free(hosts);

  msg_finalize();
  return (me == 0 && nbad) ? 1 : 0;
}
