# common.mk
# Shared build settings included by every assignment's Makefile.
# Override any of these at the top of an assignment Makefile *before*
# the `include ../common.mk` line if a given assignment needs something
# different (e.g. CC = nvcc for a CUDA assignment).

# ---- C toolchain ----
ifeq ($(origin CC),default)
CC := mpicc
endif

CFLAGS   ?= -O2 -Wall -Wextra -std=c11 -fopenmp
LDFLAGS  ?= -fopenmp -lm

# ---- LaTeX toolchain ----
LATEXMK   ?= latexmk
LATEXOPTS ?= -pdf -interaction=nonstopmode -halt-on-error -output-directory=build

# ---- Generic pattern rule for object files ----
# Assumes headers live in ./include relative to the assignment directory.
%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@
