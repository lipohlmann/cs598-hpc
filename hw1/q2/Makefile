# HW1 Q2 -- MPI ping-pong test.
#
# Deliberately standalone: ../../common.mk forces -fopenmp and assumes a flat
# src/ layout shared with the Q1 gcc build.  Q2 needs mpicc and nothing else.

CC      = mpicc
# _POSIX_C_SOURCE is needed because -std=c11 is strict ISO and would otherwise
# hide gethostname().
CFLAGS  = -O2 -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Iinclude
LDFLAGS = -lm

TARGET  = pingpong
OBJS    = src/pingpong.o src/msg_mpi.o

.PHONY: all local clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

src/%.o: src/%.c include/msg.h
	$(CC) $(CFLAGS) -c $< -o $@

# Short shared-memory smoke test on the local machine (no Slurm).
local: $(TARGET)
	./run_local.sh

clean:
	rm -f $(TARGET) $(OBJS)
