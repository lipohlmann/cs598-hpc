MPICC ?= mpicc
CFLAGS_COMMON = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -I../include
# Instruction set to compile for.  The login node (Zen 4) and the
# eng-instruction compute nodes (Zen 3 / Ice Lake) differ, so the default
# is x86-64-v3 (AVX2 + FMA), which every compute node can run.  Use
# ARCH=native only when building and running on the same kind of node.
ARCH ?= x86-64-v3
CFLAGS_OPT = -O3 -march=$(ARCH)

# BLAS for dgemm (Q2):  blis (AOCL, default), mkl, or none.
#   blis:  module load amd/aocl-aocc/aocl-aocc-5.0.0
#   mkl:   module load intel/mkl   (sets MKLROOT)
# Single-threaded libraries on purpose: one MPI rank per core.
BLAS ?= blis
ifeq ($(BLAS),blis)
  BLAS_LIBS = -lblis
else ifeq ($(BLAS),mkl)
  CFLAGS_COMMON += -Imkl_shim -I$(MKLROOT)/include
  BLAS_LIBS = -L$(MKLROOT)/lib -Wl,-rpath,$(MKLROOT)/lib \
              -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread
else
  BLAS_LIBS =
endif
LIBS = -Wl,--as-needed $(BLAS_LIBS) -lm

MAKEFLAGS += -r   # no built-in implicit rules: a config problem fails
                  # loudly instead of compiling with the wrong tool.

# Every .c file that defines main() becomes its own executable; every
# other .c file is a shared helper linked into all of them.  Nothing
# here names a specific source file, so adding/renaming files just works.
SRCS      = $(wildcard *.c)
MAIN_SRCS = $(shell grep -lE '^[[:space:]]*int[[:space:]]+main\b' $(SRCS) /dev/null)
LIB_SRCS  = $(filter-out $(MAIN_SRCS),$(SRCS))
LIB_OBJS  = $(LIB_SRCS:.c=.o)
EXES      = $(MAIN_SRCS:.c=)

all: $(EXES)

$(EXES): %: %.o $(LIB_OBJS)
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -o $@ $^ $(LIBS)

%.o: %.c $(wildcard *.h ../include/*.h)
	$(MPICC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -c $< -o $@

# Quick login-node run:  make check PROG=factor_parallel NP=4 ARGS="200 201" INPUT=8
NP    ?= 4
PROG  ?= $(firstword $(EXES))
ARGS  ?=
INPUT ?=
check: $(PROG)
	INPUT="$(INPUT)" ./run_local.sh $(NP) ./$(PROG) $(ARGS)

list:
	@echo "programs: $(EXES)"
	@echo "helpers:  $(LIB_SRCS)"

clean:
	rm -f *.o $(EXES)

.PHONY: all check list clean
