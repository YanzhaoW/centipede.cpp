## Regression with implicit equations and errors

In this section, the simple linear relation between the two variables is generalized to an implicit equation form with any number of variables and their corresponding error values.

### Implicit equation (Calibration relation)

In many cases, it's not easy to have one variable on one side of the equation and all other fitting parameters and variables on the other, while still keeping the linearity of the equation. Thus, a more general form of a linear equation, so-called _implicit equation_, would be expressed as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    f(\mathbf{x}, \mathbf{q}) = \sum^n_i x_i q_i = 0
    \label{eq:imp_func}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x}@f$ represents the variables/measurements @f$(x_0, x_1, \ldots, x_n)@f$, each of which could have different error values (standard deviation), and @f$\mathbf{q}@f$ represents fitting parameters @f$(q_0, q_1, \ldots, q_n)@f$. Note that both variables and parameters could be fixed and behave as a constant value.

### Objective function for the implicit equation

The objective function can still be obtained via the Maximal Likelihood Method, as is shown in @ref linear_objective "the previous section". The difference towards a simple linear regression is the consideration of the errors not only from just one variable, but all possible variables existing in the calibration equation. Thus, it's not enough to consider a Gaussian distribution over one single variable, but rather a multivariate Gaussian distribution. The probability of observing a set of @f$m@f$ sample points @f$(\mathbf{x}^1, \mathbf{x}^2, \ldots, \mathbf{x}^m)@f$ of independent variables @f$\mathbf{x} = (x_0, x_1, \ldots, x_n)@f$ can be described as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    Prob(\mathbf{\hat{x}}^0, \ldots, \mathbf{\hat{x}}^m, \mathbf{q}) \sim \prod^{m}_{j} \prod^{n}_{i} \exp{ -\frac{(\hat{x}^j_i)^2}{2(\sigma^j_i)^2}} 
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

subjugated by the equation @f$\eqref{eq:imp_func}@f$ for each index @f$j@f$

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation*}{
    \sum^n_i (\hat{x}^j_i + x^j_i) q_i = 0
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$x^j_i@f$ is the observed value, @f$\sigma^j_i@f$ is the corresponding error, and the @f$\hat{x}^j_i@f$, as additional fitting parameters, represents the distance between the observed value and the corresponding true value, for each variable and each sample point. This concept of introducing additional fitting parameters as the distances to their true values is the fundamental idea of the _Orthogonal Distance Regression_ (ODR)\cite boggs1989orthogonal.

A trivial choice for the objective function would just be the one following the same procedure in the linear regression, where the objective function is obtained by a negative logarithmic conversion. On the other hand, subjugated condition can be eliminated by introducing _Lagrange multipliers_ @f$\eta@f$, which leads to a formula

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \sum^m_j \sum^n_i \frac{(\hat{x}^j_i)^2}{2(\sigma^j_i)^2} + \sum^m_j \eta_j \sum^n_i (\hat{x}^j_i + x^j_i) q_i
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

However, choosing this formula as the objective function has two major drawbacks:

1. Introducing Lagrange multipliers as additional fitting parameters further increases the computation complexity.
2. The Lagrange multiplier terms cause the Hessian matrix of the objective function not to be positive definite.

The first one can be solved by set the all multiplier factors to be one very large constant value, multiplying the squared value of @f$f(\mathbf{x}, \mathbf{q})@f$, such that the subjugation still matters a lot for the minimization. The second one can be resolved by introducing _L2 regularizations_ for all fitting parameters, which essentially adds a random value to the eigen values of the Hessian matrix and force it to be positive definite. Thus, a better form of the objective function could be expressed as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \mathcal{F}(\mathbf{\hat{x}}^0, \ldots, \mathbf{\hat{x}}^m, \mathbf{q}) =& \sum_{i,\ j} \frac{(\hat{x}^j_i)^2}{2(\sigma^j_i)^2} + \frac{\eta}{2} \sum_j \left(\sum_i (\hat{x}^j_i + x^j_i) q_i\right)^2 \notag \\
   & + \frac{\lambda}{2} \left(\sum_{i,\ j} (\hat{x}^j_i)^2 + \sum_i (q_i)^2 \right)
    \label{eq:impl_objective}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Here @f$\eta@f$ and @f$\lambda@f$ are the meta parameters set before the fitting.

### First order and second order (Hessian) derivatives

As is seen from equation @f$\eqref{eq:hessian}@f$, minimization using Newton's method requires the calculation of the first order derivatives and the second order derivatives with respect to all fitting parameters. Since the objective function now depends on both the calibration parameters and the distances, equation @f$\eqref{eq:hessian}@f$ can be expressed via the matrix blocks:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \newcommand{\elementHeight}{\rule[-2em]{0pt}{4em}}
    \begin{bmatrix}
        \elementHeight \dfrac{\partial^2\mathcal{F}}{\partial q_i \partial q_{i'}} & \dfrac{\partial^2\mathcal{F}}{\partial q_i \partial \hat{x}_{i'}^{j'}} \\ 
        \elementHeight \dfrac{\partial^2\mathcal{F}}{\partial q_i \partial \hat{x}_{i'}^j} & \dfrac{\partial^2\mathcal{F}}{\partial \hat{x}_{i}^{j} \partial \hat{x}_{i'}^{j'}}
    \end{bmatrix}
    \begin{bmatrix}
        \elementHeight \delta q_i \\
        \elementHeight \delta \hat{x}^j_i
    \end{bmatrix}
    = -
    \begin{bmatrix}
    \elementHeight \dfrac{\partial\mathcal{F}}{\partial q_i} \\
    \elementHeight \dfrac{\partial\mathcal{F}}{\partial \hat{x}_i^j}
    \end{bmatrix}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Using the definition from equation @f$\eqref{eq:impl_objective}@f$, the first-order derivatives on the right-side of the equation can be calculated as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \frac{\partial\mathcal{F}}{\partial q_i} &= \lambda q_i + \eta \sum^m_j \left( \sum^n_{i'} q_{i'} (\hat{x}^j_{i'} + x^j_{i'}) \right) (\hat{x}^j_i + x^j_i) \\
    \frac{\partial\mathcal{F}}{\partial \hat{x}^j_i} &= \lambda \hat{x}_i^j + \frac{\hat{x}_i^j}{(\sigma_i^j)^2} + \eta \left( \sum_{i'}^n q_{i'} (\hat{x}^j_{i'} + x^j_{i'}) \right) q_i

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Further second-order derivatives on the left-side of the equation can be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \frac{\partial^2\mathcal{F}}{\partial q_i \partial q_{i'}} &= \delta_{ii'} \lambda + \eta \sum^m_j (\hat{x}_i^j + x_i^j)(\hat{x}_{i'}^j + x_{i'}^j)  \\
    \frac{\partial^2\mathcal{F}}{\partial q_i \partial \hat{x}_{i'}^{j'}} &= \delta_{ii'} \eta \left( \sum^n_{i'} q_{i'} (\hat{x}_{i'}^j + x_{i'}^j) \right) + \eta q_{i'} (\hat{x}_i^j + x_i^j) \\
    \frac{\partial^2\mathcal{F}}{\partial \hat{x}_{i}^{j} \partial \hat{x}_{i'}^{j'}} &= \delta_{jj'}\delta_{ii'} \left(\lambda + \frac{1}{(\sigma_i^j)^2} \right) + \delta_{jj'} \eta q_i q_{i'}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

It can be observed that the values of both of first-order and the second-order derivatives depend on the parameters themselves, unlike the case with linear regression where all derivative values are constants. Thus, it's necessary to replace the parameters values in the equation with their initial values during the first calculation of the parameter step updates, @f$\delta \mathbf{q}@f$, and repeat the same calculation using the new updated parameter values until the update values are smaller than a certain threshold.
