## Millepede algorithm

Millepede algorithm is used for reducing the dimension of the Hessian matrix if the objective function has the following prerequisites:

@anchor list_pre_req

1. Parameters can be categorized into one set of parameter group @f$\mathbf{p}@f$, called **global parameters** and other sets of parameter groups @f$\mathbf{q}_k@f$, called **local parameters**.
2. Each data point @f$(x_0, x_1, \ldots, x_n)@f$ is concerned with the global parameters and only _one_ local parameter group, i.e. the objective function for each data point must be expressed as @f$\mathcal{F}_n(\mathbf{x}, \mathbf{p}, \mathbf{q}_k)@f$. Parameters from different local parameter are not mixed with each other.
3. Only the updates on global parameters are necessary.

The resulting dimension of the factor matrix has only @f$n_p \times n_p@f$, which is independent of the number of tracks used in the calibration. The trade-off of this dimension reduction is that the step update of the global parameters cannot be obtained by just one inversion of the Hessian matrix, but rather a factor matrix calculated via a summation process of all tracks.

### Dimension reduction

The parameter update of the equation @f$\eqref{eq:hessian}@f$ can be adjusted with the introduction of local and global parameters:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \nabla^2 \mathcal{F}(\mathbf{p}, \mathbf{q}^0, \dots, \mathbf{q}^l) \delta(\mathbf{p}, \mathbf{q}^0, \dots, \mathbf{q}^l)= -\nabla\mathcal{F}(\mathbf{p}, \mathbf{q}^0, \dots, \mathbf{q}^l)
    \label{eq:obj_mille}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The first-order and the second-order derivative @f$\nabla@f$ applies to all parameters @f$(\mathbf{p}, \mathbf{q}^1, \ldots, \mathbf{q}^m)@f$. Since the local parameters from different track are not mixed, the whole objective function can be partitioned into the summation of @f$l@f$ individual functions for each track:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{F}(\mathbf{p}, \mathbf{q}_0, \dots, \mathbf{q}_l) = \sum^l_k \mathcal{F}^k(\mathbf{p}, \mathbf{q}^k)

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

If @ref list_pre_req "prerequisites" are met, it's possible to transform equation @f$\eqref{eq:obj_mille}@f$ into another equation which only updates the global parameter @f$\mathbf{p}@f$, where the dimension of the factor matrix is only equal to the number of global parameters and independent of number of tracks. The derivation of this transformation is delayed to next section and the resulting equation can be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \mathcal{C} \, \delta\mathbf{p} = - \mathbf{g}
    \label{eq:milleEq}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The _factor matrix_ @f$\mathcal{C}@f$ is a summation of two factor matrices:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{C} :=  \sum^l_k \mathcal{C}_k^1 + G_k \mathcal{C}_k^2 G_k^{\dagger}
    \label{eq:factorMat}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where the three matrices can be expressed

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align*}{

    \mathcal{C}_k^1 &= 
    \begin{bmatrix}\dfrac{\partial^2\mathcal{F}^k}{\partial p_i \partial p_{i'}}
    \end{bmatrix}_{ii',\,n_p \times n_p}
    \\
    \mathcal{C}_k^2 &= - 
    \begin{bmatrix}
    \dfrac{\partial^2\mathcal{F}^k}{\partial q_j \partial q_{j'}}
    \end{bmatrix}^{-1}_{jj',\,n_q \times n_q}
    \\
    G_k &=
    \begin{bmatrix}
    \dfrac{\partial^2\mathcal{F}^k}{\partial p_i \partial q_j}
    \end{bmatrix}_{ij,\,n_p \times n_q}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Here the indices @f$i@f$ and @f$i'@f$ represent the row or column indices for global parameters with range @f$[0, n_p)@f$ whereas @f$j@f$ or @f$j'@f$ represent the row or column indices for local parameters with range @f$[n_p, n_p + n_q)@f$. The first index occurred at the denominator represents the row index.

Thus, the factor matrix @f$\mathcal{C}@f$ needed to be inverted in the end is constructed track by track from the terms relating both Hessian matrix of global parameters and the inversion of the Hessian matrix of local parameters.

### Derivation

### Millepede on explicit calibration equation

If the calibration equation can be expressed in an explicit form, as is shown in equation @f$\eqref{eq:cal}@f$, the local parameters would just be the track related parameters representing its orientation and offset. Thus, all track have a fixed number of local parameters, @f$n_q@f$.

The total objective function would be the summation of the objective functions defined in equation @f$\eqref{eq:basic_obj}@f$ for each track. By using the first-order and the second-order derivatives of such objective function in equation @f$\eqref{eq:first_deriv}@f$ and @f$\eqref{eq:first_deriv}@f$, the factor matrices in equation @f$\eqref{eq:factorMat}@f$ become:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align*}{

    \mathcal{C}_k^1 &=  
    - \sum_{k'}^m
    \begin{bmatrix}
    \dfrac{x^{k'k}_i x^{k'k}_{i'}}{(\sigma^{k'k})^2}
    \end{bmatrix}_{ii',\,n_p \times n_p}\\
    \mathcal{C}_k^2 &=
    \left( \sum_{k'}^m
    \begin{bmatrix}
    \dfrac{x^{k'k}_j x^{k'k}_{j'}}{(\sigma^{k'k})^2}
    \end{bmatrix}_{jj',\,n_q \times n_q} \right)^{-1}\\
    G_k &=
    -\sum_{k'}^m
    \begin{bmatrix}
    \dfrac{x^{k'k}_i x^{k'k}_j}{(\sigma^{k'k})^2}
    \end{bmatrix}_{ij,\,n_p \times n_q}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

It should be explained that here the index @f$k@f$ represents the track index, @f$k'@f$ represents the data point index along the track and subscript index @f$i@f$ (or @f$i'@f$) and @f$j@f$ (or @f$j'@f$) represent the global and local parameter index respectively in the calibration equation of the data point. Thus value @f$x^{k'k}_i@f$ represents the coefficient value for @f$i@f$-th global parameter at @f$k'@f$-th equation (data point) when analyzing @f$k@f$-th track.

### Millepede on implicit calibration equation

In many cases, a clean separation of the global and local parameters isn't possible in an explicit equation. For example, if equation @f$\eqref{eq:trackeq1}@f$ becomes:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation*}{

    y_\text{track} = g_a \cdot y_\text{meas} + g_b

\f}

<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

the whole calibration relation between the two observables become:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    0 = a \cdot x_\text{meas} + b - g_a \cdot y_\text{meas} - g_b
    \label{eq:impli_eq_ex}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

By moving @f$y_\text{meas}@f$ to one side of the equation, the other side of the equation ends up a division between a global and local parameter, making the equation non-linear and not applicable for millepede algorithm.

On the other hand, observables from the experimental data all have error values and one is not more special than the other. Therefore, an implicit equation is more general to represent the calibration relations in most experiments.

With the separation of local and global parameters, the implicit calibration relation can be reformed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    f(\mathbf{x}, \mathbf{p}, \mathbf{y}, \mathbf{q}) = \sum_i^{n_g} x_i \cdot  p_i + \sum_j^{n_l} y_j \cdot q_j

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x}@f$ are observables in the multiplication of global parameters @f$\mathbf{p}@f$, and @f$\mathbf{y}@f$ are observables of local parameters @f$\mathbf{q}@f$. @f$n_g@f$ and @f$n_l@f$ are the number of global parameters and local parameters for one track.

#### Example

In case of the detector shown in @ref fig_track "figure 1" with 50 modules and each module follows has the calibration equation shown in @f$\eqref{eq:impli_eq_ex}@f$. Then, the number of global parameters and local parameters for one track are:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align*}{

    n_g &= 50\\
    n_l &= 2\\

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The x and y observables are

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align*}{

    x_i &= 
    \begin{cases}
    -y_\text{meas} & i = 2k, \quad &k \in \mathbb{Z} \\
    -1 & i = 2k + 1, \quad &k \in \mathbb{Z}
    \end{cases} \\
    y_0 &= x_\text{meas} \\
    y_1 &= 1
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

#### Factor matrices and RHS vector

The three matrices used to construct the factor matrix in equation @f$\eqref{eq:factorMat}@f$ can be evaluated using the first- and second-order derivatives shown in equations @f$\eqref{eq:impli_first_order}@f$ and @f$\eqref{eq:impli_second_order}@f$. The matrix @f$\mathcal{C}_k^1@f$ can be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{C}_k^1 =  
    \begin{bmatrix}
    \delta_{ii'}\cdot \lambda + \sum_{k'}^m ( \hat{x}^{k'k}_i + x^{k'k}_i) (\hat{x}^{k'k}_{i'} + x^{k'k}_{i'})
    \end{bmatrix}_{ii',\,n_p \times n_p}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The expression of @f$\mathcal{C}_k^2@f$ is more complicated as the distances to true values also become local parameters:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{C}_k^2 = 
    \begin{bmatrix}
        \mathcal{M}^k_{11} & \mathcal{M}^k_{12} & \mathcal{M}^k_{13} \\
        \mathcal{M}^k_{12} & \mathcal{M}^k_{22} & \mathcal{M}^k_{23} \\
        \mathcal{M}^k_{13} & \mathcal{M}^k_{23} & \mathcal{M}^k_{33} \\
    \end{bmatrix}_{jj',\,n_q \times n_q}^{-1}\\

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Here the square matrix is partitioned into three rows and columns of block matrices. The first row or column relates to the local parameter in the calibration equation. The second row or column relates to the distance parameters for global parameter coefficients while the third relates to local parameter coefficients. From the second derivatives of the ODR regression shown in equation @f$\eqref{eq:impli_second_order}@f$

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \mathcal{M}^k_{11} &=
    \begin{bmatrix}
        \delta_{ii'} \eta \left( \sum^n_{i'} p_{i'} (\hat{x}_{i'}^j + x_{i'}^j) \right) + \eta p_{i'} (\hat{x}_i^j + x_i^j)
    \end{bmatrix}_{ii'} \\
    \mathcal{M}^k_{12} &=
    \begin{bmatrix}
        \delta_{ii'} \eta \left( \sum^n_{i'} p_{i'} (\hat{x}_{i'}^j + x_{i'}^j) \right) + \eta p_{i'} (\hat{x}_i^j + x_i^j)
    \end{bmatrix}_{ii'} \\
    \mathcal{M}^k_{22} &=
    \begin{bmatrix}
        \delta_{ii'} \eta \left( \sum^n_{i'} p_{i'} (\hat{x}_{i'}^j + x_{i'}^j) \right) + \eta p_{i'} (\hat{x}_i^j + x_i^j)
    \end{bmatrix}_{ii'} \\

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->
