CC = gcc
include ../common.mk

# ---- Performance overrides for the mat-mat multiply kernel ----
# -O3                       : enable auto-vectorization of the blocked kernel
# -march=native             : use the build host's SIMD ISA (AVX2/FMA, etc.)
# -ffp-contract=fast        : fuse mul+add into FMA (off by default under -std=c11)
# -mprefer-vector-width=256 : pin the auto-vectorizer to 256-bit (AVX2) vectors,
#   matching the 6x8 register tile in mat_mat_multiply.c (sized for 16 YMM
#   registers). Without this, a host with AVX-512 could get 512-bit vectors,
#   which would overflow that tile's register budget and spill to memory.
# -fopenmp-simd             : recognize the kernel's `#pragma omp simd` hints
#   (needed so the vectorizer may reorder the tile's FP reduction -- IEEE754
#   add isn't associative, so plain -O3 refuses to vectorize it otherwise)
#   WITHOUT pulling in -fopenmp's threading runtime: it only enables SIMD
#   directives, never `#pragma omp parallel`, and links no libgomp/pthread.
CFLAGS += -O3 -march=native -ffp-contract=fast -mprefer-vector-width=256 -fopenmp-simd

# Q1 requires single-core serial code. common.mk defaults CFLAGS/LDFLAGS to
# -fopenmp for other assignments in this repo; strip it here so nothing can
# quietly reintroduce multi-threading into this kernel.
# -fopenmp was also (incidentally) what made CLOCK_MONOTONIC visible under
# -std=c11 in main.c's tic()/toc(); replace that with an explicit POSIX
# feature-test macro now that -fopenmp is gone (see q2/Makefile for the
# same pattern).
CFLAGS  := $(filter-out -fopenmp,$(CFLAGS)) -D_POSIX_C_SOURCE=199309L
LDFLAGS := $(filter-out -fopenmp,$(LDFLAGS))

# ---- Assignment-specific settings ----
TARGET  = hw1
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:.c=.o)

REPORT  = report/report.tex

.PHONY: all code pdf clean run

all: code pdf

code: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

pdf:
	$(LATEXMK) $(LATEXOPTS) -cd $(REPORT)

run: code
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	$(LATEXMK) -C -cd $(REPORT)
