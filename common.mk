# common.mk
# Shared build settings included by every assignment's Makefile.
# Override any of these at the top of an assignment Makefile *before*
# the `include ../common.mk` line if a given assignment needs something
# different (e.g. CC = nvcc for a CUDA assignment).

# ---- C toolchain ----
# Use mpicc unless CC is set on the make command line or in the assignment
# Makefile; a CC exported by the environment (e.g. the gcc module's CC=gcc)
# is ignored, since plain gcc does not link the MPI libraries.
ifneq ($(filter default environment,$(origin CC)),)
CC := mpicc
endif

CFLAGS   ?= -O2 -Wall -Wextra -std=c11 
LDFLAGS  ?=  -lm

# ---- LaTeX toolchain ----
LATEXMK   ?= latexmk
LATEXOPTS ?= -pdf -interaction=nonstopmode -halt-on-error

# ---- Generic pattern rule for object files ----
# Assumes headers live in ./include relative to the assignment directory.
%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@
