MPICC ?= mpicc
CFLAGS_COMMON = -std=c99 -D_POSIX_C_SOURCE=199309L -Wall -Wextra
# Instruction set to compile for.  "native" is only safe when the code
# runs on the same kind of cpu it was built on; the cluster's login and
# compute nodes differ, so ./driver builds with ARCH=x86-64-v3 (AVX2 +
# FMA), which every compute node can run.
ARCH ?= native
CFLAGS_OPT    = -O3 -march=$(ARCH) -ffast-math -fno-unroll-loops
LIBS = -lm

MAKEFLAGS += -r   # disable make's built-in implicit rules, so a config
                  # problem fails loudly instead of silently compiling
                  # with the wrong tool (see note in README).

# poisson.c now calls msg_wtime()/msg_rank() (see poisson.c), so
# msg_mpi.o goes into COMMON_OBJS -- every executable below links it,
# since all three share poisson.o through COMMON_OBJS.
COMMON_OBJS = block_fft.o block_fst.o transpose.o poisson.o msg_mpi.o
EXES        = test_block_fst test_poisson bench_transpose pingpong

all: $(EXES) asm

test_block_fst: $(COMMON_OBJS) test_block_fst.o
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -o $@ $^ $(LIBS)

test_poisson: $(COMMON_OBJS) test_poisson.o
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -o $@ $^ $(LIBS)

bench_transpose: $(COMMON_OBJS) bench_transpose.o
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -o $@ $^ $(LIBS)

pingpong: msg_mpi.o pingpong.o
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -o $@ $^ $(LIBS)

%.o: %.c
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -c $< -o $@

%.s: %.c
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -S $< -o $@

asm: block_fft.s block_fst.s transpose.s poisson.s

clean:
	rm -f *.o *.s $(EXES)

.PHONY: all asm clean
