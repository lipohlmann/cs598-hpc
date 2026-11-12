# CS 598 HPC Midterm

Spectral element method (GLL quadrature) code using MPI. Rank 0 builds the
mesh, quadrature set, mass matrix and stiffness matrix, then broadcasts them
to all ranks.

## Requirements

- An MPI C compiler wrapper (`mpicc`) and launcher (`mpirun`), e.g. Open MPI
- `make`
- `latexmk` (only for building the presentation)

## Build

Clone with the gslib submodule (or fetch it in an existing clone):

```sh
git submodule update --init --recursive
```

Build the code (this builds gslib first, then the `midterm` executable):

```sh
make code
```

Other targets:

```sh
make pdf        # build the presentation
make all        # code + pdf
make clean      # remove executable, objects, and LaTeX output
make distclean  # clean + remove the gslib build
```

## Run

Serial (one MPI rank):

```sh
./midterm
# or
mpirun -np 1 ./midterm
```

Parallel (e.g. 4 ranks):

```sh
mpirun -np 4 ./midterm
```

If you ask for more ranks than available cores, add `--oversubscribe`
(Open MPI):

```sh
mpirun --oversubscribe -np 8 ./midterm
```

`make run` runs the executable serially after building.

## Parameters

Domain size, number of elements per direction, and GLL points per element are
set at the top of `main()` in `src/main.c`; edit them and rebuild.
