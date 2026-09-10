CC = gcc
include ../common.mk

# ---- Performance overrides for the mat-mat multiply kernel ----
# -O3                     : enable auto-vectorization of the IKJ inner loop
# -march=native           : use the build host's full SIMD ISA (AVX2/FMA, etc.)
# -funroll-loops          : help the compiler unroll the blocked inner loops
# -ffp-contract=fast      : fuse mul+add into FMA (off by default under -std=c11)
# -mprefer-vector-width=256: pin the auto-vectorizer to 256-bit (AVX2) vectors.
#   __AVX512F__ being defined only means the ISA is targetable -- it says
#   nothing about what width the vectorizer actually emits for a plain
#   #pragma omp simd loop, and many -march=native tuning tables default to
#   256-bit anyway to dodge AVX-512 downclocking. mat_mat_multiply.c's
#   register-resident tile size is chosen assuming 256-bit vectors, so this
#   flag makes that assumption always true instead of a guess that varies
#   by GCC version/target tuning.
CFLAGS += -O3 -march=native -funroll-loops -ffp-contract=fast -mprefer-vector-width=256

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
