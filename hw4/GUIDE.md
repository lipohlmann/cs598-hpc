# Parallel Tridiagonal Solve via Schur Complement: Study Guide

Companion to your Lecture 13 notes (pages 1-14) and `schur_template.c`.
Page numbers like (p.13) refer to your handwritten notes.

---

## 1. The idea in five sentences

1. Gaussian elimination on a tridiagonal system is inherently sequential: $x_i$ needs $x_{i+1}$ (p.5).
2. But a PDE-style problem on a subdomain only needs its boundary values (p.6). If we knew the values at a few "separator" nodes, the pieces between them would decouple.
3. So pick $P-1$ separator nodes. Eliminate each block's interior **in parallel** (no communication).
4. What is left is a small system for the separator unknowns only, the **Schur complement** $S$. It is again tridiagonal and has size $(P-1)\times(P-1)$.
5. Solve $S$, then each rank recovers its interior values **in parallel** (no communication).

Communication happens only in the middle (assemble and solve $S$).

---

## 2. Layout

Your example (p.7, p.9): $n=11$, $P=3$. Blocks of $m=3$ interior nodes, separators at nodes 4 and 8.

```
node:   1  2  3 | 4 | 5  6  7 | 8 | 9 10 11
        --rank 0-- S   --rank 1-- S  --rank 2--
```

In general, with equal blocks, $n = P\,m + (P-1)$.

**Warning for your test sizes.** $n \in \{8,16,32,64\}$ do not satisfy that formula for $P=2,4$. You need **uneven blocks**. Decide how rank $p$ computes its first global index and interior size from $n$ and $P$ using integer arithmetic. Check it by hand for $n=8, P=3$ and make sure every node is owned exactly once. Require $m \ge 1$ on every rank.

Edge ranks:
- Rank 0 has no left separator.
- Rank $P-1$ has no right separator.

Handle this with `has_left` / `has_right` flags, or by giving them zero columns.

---

## 3. Block structure (p.8-10)

Reorder unknowns as (all interiors, then all separators):

$$
\begin{pmatrix} A_I & U \\ L^T & D \end{pmatrix}
\begin{pmatrix} x_I \\ x_S \end{pmatrix}
=
\begin{pmatrix} f_I \\ f_S \end{pmatrix}
$$

- $A_I = \mathrm{blockdiag}(A_1,\dots,A_P)$: each $A_p$ is an ordinary tridiagonal matrix (p.8).
- $D$ is diagonal with entries $a_{\text{sep}}$ (p.9), because separators are never adjacent.
- $U$ (interior rows, separator columns) and $L^T$ (separator rows, interior columns) are extremely sparse. For rank $p$ only **two columns of $U$** and **two rows of $L^T$** are nonzero (p.10).

Block Gaussian elimination (p.8):

$$
S = D - L^T A_I^{-1} U, \qquad
\begin{pmatrix} A_I & U \\ 0 & S \end{pmatrix}
\begin{pmatrix} x_I \\ x_S \end{pmatrix}
=
\begin{pmatrix} f_I \\ f_S - L^T A_I^{-1} f_I \end{pmatrix}
$$

Because $A_I$ is block diagonal, $L^T A_I^{-1} U$ is a **sum of independent per-rank terms**, so rank $p$ computes only its own piece.

### What are the four nonzero vectors on rank $p$?

Let the block have interior rows $1,\dots,m$, left separator $s_L$, right separator $s_R$. In the global matrix:

| Vector | Meaning | Only nonzero at | Initial value |
|---|---|---|---|
| $u^{L}$ (column of $U$) | coupling interior rows to left separator | row 1 | $c_1$ (the subdiagonal entry of the first interior row) |
| $u^{R}$ (column of $U$) | coupling interior rows to right separator | row $m$ | $b_m$ (the superdiagonal entry of the last interior row) |
| $\ell^{L}$ (row of $L^T$) | left separator's row, interior part | column 1 | $b_{s_L}$ |
| $\ell^{R}$ (row of $L^T$) | right separator's row, interior part | column $m$ | $c_{s_R}$ |

Check this against p.9 and p.10: separator 4 has $c_4$ (to $x_3$) and $b_4$ (to $x_5$). Column 4 has $b_3$ and $c_5$.

---

## 4. The local sweep (p.13 "Main Loop")

This is ordinary Thomas elimination (p.3) on the block, **dragging along** the two extra columns and two extra rows. Each line of the loop is one row operation.

At step $k$ (for $k=1,\dots,m-1$), pivot row $k$ eliminates column $k$ from row $k+1$ **and** from the two separator rows.

| Notes' line | What it does | Why |
|---|---|---|
| `pinv = 1/a_k` | reciprocal of pivot | one division, then only multiplies (p.4) |
| `c_{k+1} = pinv*c_{k+1}` | multiplier $c'_{k+1}$ | same as Thomas (p.3) |
| `a_{k+1} -= c_{k+1} b_k` | update diagonal | Thomas |
| `f_{k+1} -= c_{k+1} f_k` | update rhs | Thomas |
| `u_{k+1} = 0 - c_{k+1} u_k` | **fill** in the left-separator column | row $k+1$ minus multiple of row $k$ where row $k$ has $u_k$ and row $k+1$ had a zero |
| `l_k = pinv * l_k` | multiplier for the separator row | eliminate its column-$k$ entry |
| `l_{k+1} = -l_k b_k` | **fill** in the separator row | pivot row $k$ has $b_k$ in column $k+1$ |
| `S11 -= l_k u_k` | update separator diagonal | pivot row $k$ has $u_k$ in the separator column |
| `fS1 -= l_k f_k` | update separator rhs | same row operation on the rhs |

Questions to answer for yourself before coding:

1. **Order of operations.** Which values of $u_k$, $f_k$, $a_k$ must be used in iteration $k$: the original or the already-updated ones? (Draw the $2\times2$ case.)
2. **Why does only the *left* column and *left* row fill in?** Use the key observation on p.11: no fill until the row index passes the first nonzero of the column. Then explain what happens for $u^R$ and $\ell^R$, which are nonzero only in position $m$.
3. What must you **store** after the sweep so the back-substitution can run later? (Hint: p.4 and the fill vectors.)

---

## 5. Finishing the last row (p.14, "II Schur complement update")

After the loop, row $m$ is the last pivot row. Eliminate column $m$ from **both** separator rows using it:

- Form the multipliers $\ell^L_m/a_m$ and $\ell^R_m/a_m$.
- Update the $2\times2$ local Schur block:
  - $S_{11}$ uses $\ell^L$ and $u^L$
  - $S_{12}$ uses $\ell^L$ and $u^R$
  - $S_{21}$ uses $\ell^R$ and $u^L$
  - $S_{22}$ uses $\ell^R$ and $u^R$
- Update the two local rhs entries $f_{S1}, f_{S2}$ with $\tilde f_m$.

Initial values (p.13): $S_{11}=0$, $S_{22}=a_{s_R}$, $f_{S1}=0$, $f_{S2}=f_{s_R}$. The separator's diagonal is **split**: its $a$ is added once (by the left-hand rank), and both neighbours add their contributions.

Question: why is $S_{12}$ nonzero here but both off-diagonals of the *interior* part never mattered?

---

## 6. Assembling and solving $S$ (p.12, p.14)

Separator $q$ ($q=1,\dots,P-1$) gets contributions from rank $q$ (its $S_{22}$, $f_{S2}$) and rank $q+1$ (its $S_{11}$, $f_{S1}$). The $S_{12},S_{21}$ of a rank are the off-diagonal entries between neighbouring separators.

Two options:

- **Simple.** Gather all local $2\times 2$ pieces on every rank (e.g. `MPI_Allgather`), assemble the $(P-1)$-sized tridiagonal system, solve it redundantly on every rank with your own Thomas routine. Fine for $P \le$ hundreds.
- **Notes' version (p.14).** Each rank exchanges its $S_{22}, f_{S2}$ with its right neighbour and sums, giving 2 messages total per rank. Then $S$ is still distributed (one row per rank) and needs a distributed or gathered tridiagonal solve.

Start with the simple one.

---

## 7. Back-substitution

Once every rank knows $x_{s_L}$ and $x_{s_R}$, recover the interior using the stored factors. For $k=m,\dots,1$:

$$
x_k = \frac{1}{a'_k}\Big(f'_k - b_k\,x_{k+1} - u^L_k\,x_{s_L} - u^R_k\,x_{s_R}\Big)
$$

Compare with the formula on p.4. Be careful about which terms are absent at $k=m$ (hint: where did $b_m$ go?). This step needs **no** communication.

---

## 8. MPI primer (only what you need)

```c
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&p);      // my id, 0..P-1
MPI_Comm_size(MPI_COMM_WORLD,&P);      // total number of processes
...
MPI_Finalize();
```

Mental model: you launch $P$ copies of the **same program**. They differ only in the value of `p`. Variables are private to each copy. Data moves only through MPI calls.

| Call | Use here |
|---|---|
| `MPI_Sendrecv` | exchange with a neighbour without deadlock |
| `MPI_Allgather` / `MPI_Allreduce` | share the small local $S$ pieces with everyone |
| `MPI_Barrier`, `MPI_Wtime` | timing |
| `MPI_Gatherv` | collect $x$ on rank 0 for checking |

Deadlock warning: if every rank calls blocking `MPI_Send` to its neighbour first, they can all wait forever. `MPI_Sendrecv` avoids that.

Build and run:

```
mpicc -O2 -Wall -o solver schur_template.c -lm
mpirun -np 4 ./solver 64
```

---

## 9. Suggested roadmap (do these in order)

**Stage 0: serial Thomas** (p.3-4). Implement factor and solve, test with the residual $\|Ax-f\|_\infty$. You will reuse it for the Schur system.

**Stage 1: fake the parallelism in a single process.** Write `local_sweep` as a function that takes one block's data. Call it in a `for (p = 0; p < P; p++)` loop, assemble $S$ in ordinary arrays, solve it, loop again for back-substitution. No MPI yet. Compare with Stage 0. Debug here. It is dramatically easier than debugging MPI.

**Stage 2: hand check.** Do $n=7$, $P=2$ (p.7) on paper, with all $a_i=2$, $b_i=c_i=-1$, $f_i=1$. Compare your program's intermediate $S$.

**Stage 3: add MPI.** Replace the `for p` loop by "each rank does only its own $p$." Replace the "assemble in arrays" by `MPI_Allgather`.

**Stage 4: test.** Many $n$ and $P$ (including uneven, including $P=1$). Residual should be near $10^{-15}$. Then time larger $n$ and plot speedup.

---

## 10. Pitfalls checklist

- Off-by-one between 0-based C arrays and the 1-based notes. Decide up front and write it in a comment.
- The multiplier overwrites $c_{k+1}$ in place. Do not use the old $c_{k+1}$ afterwards.
- Edge ranks: no left separator (rank 0), no right separator (rank $P-1$).
- A block with $m=1$: the loop $k=1,\dots,m-1$ runs zero times. Does the rest still work?
- All ranks must call collective operations (`Allgather`, `Barrier`), or the program hangs.
- Every rank must generate the **same** random matrix if you slice it from a global one (same seed).
- Print from rank 0 only, or output interleaves.

---

## 11. Self-check questions

1. Why does this need $A$ SPD (or diagonally dominant)? What would happen to the separator elimination with pivoting?
2. How many floating-point operations does the parallel version do compared with serial Thomas? Where does the extra work come from, and what does that imply for speedup?
3. What limits scaling as $P \to n$?
4. Why is $S$ tridiagonal? (Draw which separators share a block.)
