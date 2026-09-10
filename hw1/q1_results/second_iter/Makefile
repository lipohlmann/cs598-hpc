CC = gcc
include ../common.mk

# ---- Performance overrides for the mat-mat multiply kernel ----
# -O3               : enable auto-vectorization of the IKJ inner loop
# -march=native     : use the build host's full SIMD ISA (AVX2/FMA, etc.)
# -funroll-loops    : help the compiler unroll the blocked inner loops
# -ffp-contract=fast: fuse mul+add into FMA (off by default under -std=c11)
CFLAGS += -O3 -march=native -funroll-loops -ffp-contract=fast

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
