# Theoretical background {#linear_reg}

[TOC]

The millepede algorithm is fundamentally a linear regression model, which minimizes the _objective function_ with respect to parameters in a linear equation, based on a set of available data points. In this page, the traditional linear regression is explained, followed by a revised version better suited for millepede algorithm and the real data characteristics.

## Simple linear regression

### Objective functions

In the simplest case of linear regression on a calibration equation:

<!-- prettier-ignore-start -->
\f{equation}{
y = f(x, a, b) = a \cdot x + b
\f}
<!-- prettier-ignore-end -->

where @f$(x, y)@f$ are available data points with parameters @f$a@f$ and @f$b@f$ that need to be optimized. The first-principle approach of parameter optimization is through the so-called _Maximum Likelihood Estimation_, which assumes that @f$y@f$ values follow the normal distribution and the probability to get such @f$y@f$ values being observed is at its maximum when parameters are opitimized. Thus, the probability of the observed data sets @f$(x_i, y_i)@f$ can be express as:

<!-- prettier-ignore-start -->
\f{equation}{
Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}
\label{eq:MLE}
\f}
<!-- prettier-ignore-end -->

where @f$\mathbf{x}@f$ and @f$\mathbf{y}@f$ represents observed data values @f$(x_0, x_1, \ldots, x_n)@f$ and @f$(y_0, y_1, \ldots, y_n)@f$ respectively, and @f$\sigma_i@f$ represents the @f$y@f$ error of @f$i@f$-th data point. Maximizing the formula above is equally maximizing its logarithmic transformed formula:

<!-- prettier-ignore-start -->
\f{align}{
\argmax{a, b} \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}} &= \argmax{a, b} \log\left(\prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}\right) \notag \\
&= \argmax{a, b} \sum^{n}_{i} -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2} \notag \\
&= \argmin{a, b} \sum^{n}_{i}\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}
\f}
<!-- prettier-ignore-end -->

Thus, a probability maximization turns into the minimization of a function, which is defined as the **objective function**. By defining another term, called **residual**, denoted as

<!-- prettier-ignore-start -->
\f{equation}{
z = \bar{y} - f(\bar{x}, a, b)
\label{eq:residual}
\f}
<!-- prettier-ignore-end -->

where @f$\bar{x}@f$ and @f$\bar{y}@f$ are both observed data, the minimization can then be expressed as:

<!-- prettier-ignore-start -->
\f{equation}{
\hat{a}, \hat{b} = \argmin{a, b} \sum^{n}_{i}\frac{z(x_i, y_i, a, b)^2}{2\sigma_i^2}
\f}
<!-- prettier-ignore-end -->

In general cases where @f$m@f$ parameters need to be optimized, denoted as @f$\mathbf{p} = (p_0, p_1, \ldots, p_m)@f$, the _objective function_ is expressed as:

<!-- prettier-ignore-start -->
\f{equation}{
\mathcal{F}(\mathbf{p}) = \sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2}
\f}
<!-- prettier-ignore-end -->

where @f$f(x_i, \mathbf{p})@f$ can be any functions, including non-linear ones in parameters @f$\mathcal{p}@f$.

### Minimization process

The minimization process in this project is using so-called _Gaussian Newton's Method_ \cite wiki:GausNewton, which basically assumes the linear relations of the optimization parameters in the calibration equation:

<!-- prettier-ignore-start -->
\f{equation}{
f(x, \mathbf{p}) = f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} + \nabla_{\mathbf{p}} f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \cdot(\mathbf{p} - \mathbf{p}_\text{init})
\f}
<!-- prettier-ignore-end -->

Thus, the iteration update on the parameters @f$\mathbf{p}@f$, which is derived from the traditional _Newton's Method_, can be further simplified as:

<!-- prettier-ignore-start -->
\f{flalign}{
   & & \nabla_{\mathbf{p}}^2 \mathcal{F} \, \delta \mathbf{p} &= \nabla_{\mathbf{p}} \mathcal{F} \notag & \\
   &\implies & \nabla_{\mathbf{p}}^2 \left(\sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2} \right) \, \delta\mathbf{p} &= \nabla_{\mathbf{p}} \left( \sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2} \right) \notag & \\
   &\implies & \left(\sum^{n}_i \frac{\nabla_{\mathbf{p}} f(x_i, \mathbf{p}) \nabla_{\mathbf{p}}^{\dagger} f(x_i, \mathbf{p})}{\sigma_i^2} \right) \, \delta\mathbf{p} &= - \sum^{n}_i \frac{z(x_i, y_i, \mathbf{p}) \nabla_{\mathbf{p}} f(x_i, \mathbf{p})}{\sigma_i^2} &\\
\f}
<!-- prettier-ignore-end -->

where @f$z(x_i, y_i, \mathbf{p})@f$ is defined from equation @f$\eqref{eq:residual}@f$. The left-hand side matrix, @f$\nabla_{\mathbf{p}}^2 \mathcal{F}@f$ is called _Hessian matrix_, which is simplified to be the external product of the gradient of the calibration function@f$f(x, \mathbf{p})@f$ in the Gaussian Newton's method. There are four prerequisites for a successful minimization:

1. Initial values of all parameters @f$\mathbf{p}@f$ must be available.
2. Error values @f$\sigma_i@f$ must not be zero.
3. Hessian matrix on the left-hand side must not have rank-deficit and must be invertible.
4. Hessian matrix on the left-hand side must be positive definite.

Fortunately, a majority of the calibration equations concerning real-world detectors already fulfilled these prerequisites, except the third one, which can be easily fixed by fixing some parameters (see section). Nevertheless, there are two more issues when applying this minimization to the real experimental data:

* Both @f$x@f$ and @f$y@f$ have error values. Ignoring @f$x@f$ error values could cause inaccuracies on the optimized parameter values.
* Number of parameters could grow very large when the calibration involves with data from lots of tracks. At some points, trying to invert the Hessian matrix becomes impractical and very expensive.

Thus, in the next sessions, several methods addressing these issues will be explained in detail.

## Regression with both x and y errors

To introduce @f$x@f$ errors both in the objective function and minimization step, some changes are required when calculating the maximum probability in equation @f$\eqref{eq:MLE}@f$. Assuming both @f$x@f$ and @f$y@f$ values follow the Gaussian distribution and they are independent, the probability for the observed data points can be expressed as:

<!-- prettier-ignore-start -->
\f{equation}{
    Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{n}_{i} \exp{ -\frac{\left(y_i - f(\mu_i, \mathbf{p})\right)^2}{2(\sigma^y_i)^2}} \exp{ -\frac{\left(x_i - \mu_i\right)^2}{2(\sigma^x_i)^2}}
\f}
<!-- prettier-ignore-end -->

where @f$\mu_i@f$ represents the true @f$x@f$ values and @f$f(\mu_i, \mathbf{p})@f$ the true @f$y@f$ values. Following a similar derivation, the objective function can then be expressed as

<!-- prettier-ignore-start -->
\f{equation}{
    \mathcal{F}(\mathbf{p}, \boldsymbol{\mu}) = \sum^{n}_{i}\left(\frac{\left(y_i - f(\mu_i, \mathbf{p})\right)^2}{2(\sigma^y_i)^2} + \frac{\left(x_i - \mu_i\right)^2}{2(\sigma^x_i)^2} \right)
\f}
<!-- prettier-ignore-end -->

It can be seen here that the @f$x@f$ true values @f$\boldsymbol{\mu}@f$ are treated as additional optimization parameters and its form is exactly the same as the objective function derived in the _Orthogonal Distance Regression_ (ODR)\cite boggs1989orthogonal.
