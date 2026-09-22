## Basic linear regression

To understand the Millepede algorithm, it's important to know how the linear regression works ab initio and why certain calculations exist in the fitting process.

### Objective functions and Maximum Likelihood Estimation {#linear_objective}

In the simplest case of linear regression:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    y = f(\mathbf{x}, \mathbf{p}) = \sum^n_i x_i p_i
    \label{eq:cal}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$(\mathbf{x}, y)@f$ is a set of measurable variables with fitting parameters @f$\mathbf{p} = (p_0, p_1, \ldots, p_m)@f$. Such an equation is often referred to as the _explicit_ equation where only variable @f$y@f$ is on one side of the equation and all other variables and parameters on the other side. It's also required that only variable @f$y@f$ has uncertainty. The first-principle approach of parameter optimization is through the so-called _Maximum Likelihood Estimation_, which assumes that @f$y@f$ value follows a normal distribution and, if all parameters are optimized, the maximum probability is achieved with the @f$y@f$ value being the observed value. The probability of the @f$m@f$ observed data sets @f$(\mathbf{x}^j, y^j)@f$ can be express as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{m}_{j}\exp{ -\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2}}
    \label{eq:MLE}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x}^j@f$ represents measurable values @f$(x^j_0, x^j_1, \ldots, x^j_n)@f$ without uncertainty at @f$j@f$-th data point, and @f$\sigma^j@f$ represents the uncertainty of the corresponding measurable @f$y^j@f$. Maximizing the formula above is equally maximizing its logarithmic transformed formula:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \argmax{\mathbf{p}} \prod^{m}_{j}\exp{ -\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2}} &= \argmax{\mathbf{p}} \log\left(\prod^{m}_{j}\exp{ -\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2}}\right) \notag \\
    &= \argmax{\mathbf{p}} \sum^{m}_{j} -\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2} \notag \\
    &= \argmin{\mathbf{p}} \sum^{m}_{j}\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Thus, a probability maximization turns into the minimization of a function, the so-called **objective function**. By defining the **residual** term as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    z = y - f(\mathbf{x}, \mathbf{p})

\label{eq:residual}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

the minimization can also be expressed in terms of residual values:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \hat{\mathbf{p}} = \argmin{\mathbf{p}} \sum^{m}_{j}\frac{(z^j)^2}{2(\sigma^j)^2}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The _objective function_ can be expressed as a function depending on the fitting parameters @f$\mathbf{p}@f$:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{F}(\mathbf{p}) = \sum^{m}_{j}\frac{(z^j)^2}{2(\sigma^j)^2} = \sum^{m}_{j}\frac{\left(y^j - f(\mathbf{x}^j, \mathbf{p})\right)^2}{2(\sigma^j)^2}
    \label{eq:basic_obj}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

This objective is still valid even if @f$f(\mathbf{x}, \mathbf{p})@f$ is not linearly dependent on the fitting parameters @f$\mathbf{p}@f$.

### Minimization process via Newton's method

The minimization process in this project is using so-called _Newton's Method_ \cite wiki:GaussNewton, which approximates the objective function up to its second order of its Taylor expansion:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \mathcal{F}(\mathbf{p}) \sim \mathcal{F}(\mathbf{p}) \bigg\rvert_{\mathbf{p}_\text{init}} + \nabla_{\mathbf{p}} \mathcal{F}(\mathbf{p}) \bigg\rvert_{\mathbf{p}_\text{init}} (\mathbf{p} - \mathbf{p}_\text{init}) + (\mathbf{p} - \mathbf{p}_\text{init})^{\dagger} \, \nabla_{\mathbf{p}}^2 \mathcal{F}\bigg\rvert_{\mathbf{p}_\text{init}} (\mathbf{p} - \mathbf{p}_\text{init})
    \label{eq:taylor}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Thus, the minimum of this second order function can be found by updating the parameter values by @f$\delta \mathbf{p}@f$:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \nabla_{\mathbf{p}}^2 \mathcal{F}\bigg\rvert_{\mathbf{p}_\text{init}} \, \delta \mathbf{p} = - \nabla_{\mathbf{p}} \mathcal{F}\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}}
    \label{eq:hessian}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The left-hand side _factor matrix_, @f$\nabla_{\mathbf{p}}^2 \mathcal{F}@f$, is called _Hessian matrix_, which is the second-order partial derivative with respect to @f$p_i@f$ and @f$p_{i'}@f$:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \left[\nabla_{\mathbf{p}}^2 \mathcal{F}\right]_{ii'} = \frac{\partial^2 \mathcal{F}}{\partial p_i \partial p_{i'}}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

and the vector on the right-hand side of the equation is typically called _right-hand side vector_.

There are four prerequisites for a successful minimization:

1. Initial values of all parameters @f$\mathbf{p}@f$ must be available.
2. Error values @f$\sigma_i@f$ must not be zero.
3. Hessian matrix on the left-hand side must not have rank-deficit and must be invertible.
4. Hessian matrix on the left-hand side must be positive definite.

The approximation up to the second-order terms may not be accurate for all possible parameter values. Therefore, one iteration may not be enough to determine the real minima of the objective function and multiple iteration needs to be done using the new initial values from the parameter update of the last iteration. The iteration ends when the parameter update is below a certain threshold.

### Evaluation of the parameter update

Equation @f$\eqref{eq:hessian}@f$ can be expressed in a matrix form:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \newcommand{\elementHeight}{\rule[-2em]{0pt}{4em}}
    \begin{bmatrix}
        \elementHeight\dfrac{\partial^2 \mathcal{F}}{\partial p_i \partial p_{i'}}
    \end{bmatrix}
    \begin{bmatrix}
        \elementHeight \delta p_i
    \end{bmatrix}
    =-
    \begin{bmatrix}
        \elementHeight\dfrac{\partial \mathcal{F}}{\partial p_i}
    \end{bmatrix}


\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The first-order derivative on the right-side of the equation can be evaluated by using the definition from equation @f$\eqref{eq:basic_obj}@f$:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{

    \dfrac{\partial \mathcal{F}}{\partial p_i} &= \sum_j^m \frac{\left(y^j - \sum^n_{i'} (x^j_{i'} p_{i'})\right)x^j_i}{(\sigma^j)^2} \notag \\
    &= \sum_j^m \frac{z^j x^j_i}{(\sigma^j)^2}
    \label{eq:first_deriv}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The further second-order derivative can be evaluated as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    \dfrac{\partial^2 \mathcal{F}}{\partial p_i \partial p_{i'}} = - \sum_j^m \frac{x^j_i x^j_{i'}}{(\sigma^j)^2}
    \label{eq:second_deriv}

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

From these derivative values, it can be observed that

1. The second-order derivative are constants, independent of any fitting parameters, which leads to any higher order terms in the Taylor expansion to be zero. Therefore, equation @f$\eqref{eq:taylor}@f$ represents the exact objective function and one iteration is enough to determine the true minimum of the objective function.

2. The Hessian matrix is guaranteed positive definite, as it's in a form of @f$X^\dagger X@f$.
