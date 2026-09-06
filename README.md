# CS 598: High-Performance Parallel Computing, Fall 2026

Collaborative repo for graduate high-performance parallel computing homework.
Each assignment is C code + a LaTeX writeup, built with plain `make`.

## Layout

```
hpc-hw/
├── common.mk          # shared compiler flags + LaTeX rule, included by every Makefile
├── hw-template/        # copy this to start a new assignment
│   ├── Makefile
│   ├── src/            # .c files
│   ├── include/        # .h files
│   ├── report/
│   │   ├── report.tex
│   │   └── figures/
│   └── data/            # optional input files
├── hw1/, hw2/, ...     # one directory per assignment, same layout
└── .gitignore
```

## Starting a new assignment

```sh
cp -r hw-template hw3
cd hw3
# edit Makefile: change TARGET = hw-template -> TARGET = hw3
```

## Building

From inside an assignment directory:

| Command      | Effect                                   |
|--------------|-------------------------------------------|
| `make`       | builds the C binary **and** the PDF report |
| `make code`  | builds only the C binary                  |
| `make pdf`   | builds only the report (via `latexmk`)    |
| `make run`   | builds (if needed) and runs the binary    |
| `make clean` | removes binary, object files, LaTeX build cruft |

## Requirements

- `gcc`/`mpicc` (or whatever `CC` you set — see below)
- `latexmk` (part of most TeX distributions, e.g. TeX Live)

## Per-assignment overrides

If an assignment doesn't need MPI, or needs CUDA, override the toolchain at
the **top** of that assignment's `Makefile`, before the `include`:

```makefile
CC = gcc          # plain C, no MPI
# or
CC = nvcc         # CUDA
include ../common.mk
```

Everything else (object-file rule, LaTeX rule) is inherited from
`common.mk` automatically.
