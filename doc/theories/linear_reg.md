# Linear regression {#linear_reg}

[TOC]

The millepede algorithm is fundamentally a linear regression model, which minimizes the _objective function_ with respect to parameters in a linear equation, based on a set of available data points. In this page, the traditional linear regression is explained, followed by a revised version better suited for millepede algorithm and the real data characteristics.

## Objective functions

In the simplest case of linear regression on a calibration equation:

\f{equation}{
y = f(x, a, b) = a \cdot x + b
\f}

where @f$(x, y)@f$ are available data points with parameters @f$a@f$ and @f$b@f$ that need to be optimized. The first-principle approach of parameter optimization is through the so-called _Maximum Likelihood Estimation_, which assumes that @f$y@f$ values follow the normal distribution and the probability to get such @f$y@f$ values being observed is at its maximum when parameters are opitimized. Thus, the probability of the observed data sets @f$(x_i, y_i)@f$ can be express as:

\f{equation}{
Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}
\f}

where @f$\mathbf{x}@f$ and @f$\mathbf{y}@f$ represents observed data values @f$(x_0, x_1, \ldots, x_n)@f$ and @f$(y_0, y_1, \ldots, y_n)@f$ respectively, and @f$\sigma_i@f$ represents the @f$y@f$ error of @f$i@f$-th data point. Maximizing the formula above is equally maximizing its logarithmic transformed formula:

\f{align}{
\argmax{a, b} \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}} &= \argmax{a, b} \log\left(\prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}\right) \notag \\
&= \argmax{a, b} \sum^{n}_{i} -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2} \notag \\
&= \argmin{a, b} \sum^{n}_{i}\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}
\f}

Thus, a probability maximization turns into the minimization of a function, which is defined as the **objective function**. By defining another term, called **residual**, denoted as

\f{equation}{
z = \bar{y} - f(\bar{x}, a, b)
\f}

where @f$\bar{x}@f$ and @f$\bar{y}@f$ are both observed data, the minimization can then be expressed as:

\f{equation}{
\hat{a}, \hat{b} = \argmin{a, b} \sum^{n}_{i}\frac{z(x_i, y_i, a, b)^2}{2\sigma_i^2}
\f}

In general cases where @f$m@f$ parameters need to be optimized, denoted as @f$\mathbf{p} = (p_0, p_1, \ldots, p_m)@f$, the _objective function_ is expressed as:

\f{equation}{
\mathcal{F}(\mathbf{p}) = \sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2}
\f}

where @f$f(x_i, \mathbf{p})@f$ can be any functions, including non-linear ones in parameters @f$\mathcal{p}@f$.

## Minimization process

The minimization process in this project is using so-called _Gaussian Newton's Method_ \cite wiki:GausNewton, which basically assumes the linear relations of the optimization parameters in the calibration equation:

\f{equation}{
f(x, \mathbf{p}) = f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} + \nabla_{\mathbf{p}} f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \cdot(\mathbf{p} - \mathbf{p}_\text{init})
\f}
