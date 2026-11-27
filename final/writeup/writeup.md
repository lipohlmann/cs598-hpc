---
title: "Midterm Writeup: Matrix-Free Spectral Elements in 3D on Elliptic Problem"
authors:
  - name: Liam Pohlmann
    affiliation: 1
  - name: Alex Shaffer
    affiliation: 1
affiliations:
  - name: University of Illinois at Urbana-Champaign, United States
    index: 1
date: Fall 2026
header-includes:
  - \usepackage{amsmath}
  - \usepackage{amssymb}
  - \usepackage{physics}
  - \usepackage{geometry}
  - \usepackage{hyperref}
  - \usepackage[noabbrev,capitalize]{cleveref}
---

In this project, we implemented a parallel, matrix-free spectral element solver for a 3D elliptic problem on the unit cube $\Omega=[0,1]^3$.
The model problem is

\begin{equation}\label{eq:pde}
-\nabla^2 u + u = f \ \text{ in } \Omega, \qquad u = 0 \ \text{ on } \partial\Omega.
\end{equation}

To test for convergence, we preselected the manufactured solution

$$
u(x,y,z) = \sin(4\pi x)\sin(2\pi y)\sin(3\pi z),
$$

which vanishes on all faces of the cube.
Since $-\nabla^2 u = (4^2+2^2+3^2)\pi^2 u = 29\pi^2 u$, this leads to the forcing function

$$
f(x,y,z) = (29\pi^2 + 1)\,u(x,y,z).
$$

Multiplying \cref{eq:pde} by a test function $v$ with $v=0$ on $\partial\Omega$, integrating over the domain, and integrating by parts (the boundary term vanishes because $v=0$ on $\partial\Omega$) gives the weak form

$$
\int_\Omega \nabla u\cdot\nabla v \,dV + \int_\Omega u\,v \,dV = \int_\Omega f\,v \,dV .
$$

With the spectral element method (SEM), we partition the domain into $E$ non-overlapping elements and require the weak form to hold over the whole domain, assembled as a sum of element contributions with $C^0$ continuity across element faces.
The integrals are approximated with Gauss-Lobatto-Legendre (GLL) quadrature.
The GLL nodes on the reference interval also serve as interpolation nodes for the Lagrange polynomials $\ell_i$ that form the basis of the spectral expansion on each element:

$$
u(x,y,z) = \sum_{k=0}^{N_z}\sum_{j=0}^{N_y}\sum_{i=0}^{N_x}u_{ijk}\,\ell_i(x)\ell_j(y)\ell_k(z).
$$

The test function is expanded in the same basis (a Galerkin method).
Using Kronecker product notation, the element-local system is

$$
\begin{aligned}
& \mathbf{v}^T_e\left(\hat{M}_z\otimes \hat{M}_y\otimes \hat{K}_x\right)\mathbf{u}_e+
    &\mathbf{v}^T_e\left(\hat{M}_z\otimes \hat{K}_y\otimes \hat{M}_x\right)\mathbf{u}_e+\\+
    &\mathbf{v}^T_e\left(\hat{K}_z\otimes \hat{M}_y\otimes \hat{M}_x\right)\mathbf{u}_e+
    &\mathbf{v}^T_e\left(\hat{M}_z\otimes\hat{M}_y\otimes\hat{M}_x\right)\mathbf{u}_e=\\=
    &\mathbf{v}^T_e\left(\hat{M}_z\otimes\hat{M}_y\otimes\hat{M}_x\right)\mathbf{f}_e.
\end{aligned}
$$

where $\hat K$ and $\hat M$ are the 1D stiffness and mass matrices, respectively, including the element-size scaling (approximately $2/h$ for $\hat K$ and $h/2$ for $\hat M$ in each direction on affine elements).
Under GLL quadrature the mass matrices are diagonal.
The global operator is $A = Q^T A_L Q$, where $A_L$ is block-diagonal over elements and $Q$ is the scatter operator that copies global node values to element-local storage; $Q^T$ is the corresponding gather (sum).
Homogeneous Dirichlet nodes are handled by masking.

Applying each Kronecker product as a sequence of small 1D tensor contractions (sum-factorization) reduces the operation count per element from $O(N^6)$ for a dense element matrix to $O(N^4)$, and the storage from $O(N^6)$ to $O(N^3)$.

For parallelization, the unit-cube domain was distributed using Recursive Coordinate Bisection (RCB) to achieve load balancing.
The gather/scatter operations, which are addition/copy routines used to communicate and update node values shared across processors, were handled by [gslib](https://github.com/nek5000/gslib).
Because the system is symmetric positive definite, a straightforward parallel conjugate gradient (CG) solver was implemented
This algorithm is particularly well-suited for SEM because it only requires the *action* of a matrix on a vector, and it only requires a couple global reductions per iteration.

Our implementation achieved exponential (spectral) convergence in a $p$-type convergence study.
In an $h$-type convergence study with polynomial degree $N=3$ and $N=5$, we observed fourth- and sixth-order convergence, respectively, in the $L_2$ norm, consistent with the expected $O(h^{N+1})$ rate in $L^2$.
On a strong-scaling study (43986977 d.o.f.s), $>80\%$ parallel efficiency was maintained up to about 64 MPI ranks, after which communication dominated.
A similar trend was seen in a weak-scaling study (\~704969 d.o.f.s).
As the number of ranks grew, the fraction of time spent in the Kronecker product evaluations, the most computationally expensive part of the algorithm, fell below the time spent in `Allreduce` and became comparable to that of the gather-scatter routine.
This indicates that at high rank counts the collective communication in CG, rather than the local tensor-product kernels, limits scalability.

