# Theoretical background {#linear_reg}

[TOC]

The millepede algorithm is fundamentally a linear regression model, which minimizes the _objective function_ with respect to parameters in a linear equation, based on a set of available data points. In this page, the traditional linear regression is explained, followed by a revised version better suited for millepede algorithm and the real data characteristics.

## Simple linear regression

### Objective functions

In the simplest case of linear regression on a calibration equation:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
y = f(x, a, b) = a \cdot x + b
\label{eq:cal}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$(x, y)@f$ is available data points with parameters @f$a@f$ and @f$b@f$ that need to be optimized. The first-principle approach of parameter optimization is through the so-called _Maximum Likelihood Estimation_, which assumes that @f$y@f$ values follow the normal distribution and the probability to get such @f$y@f$ values being observed is at its maximum when parameters are optimized. Thus, the probability of the observed data sets @f$(x_i, y_i)@f$ can be express as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}
\label{eq:MLE}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x}@f$ and @f$\mathbf{y}@f$ represents observed data values @f$(x_0, x_1, \ldots, x_n)@f$ and @f$(y_0, y_1, \ldots, y_n)@f$ respectively, and @f$\sigma_i@f$ represents the @f$y@f$ error of @f$i@f$-th data point. Maximizing the formula above is equally maximizing its logarithmic transformed formula:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{align}{
\argmax{a, b} \prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}} &= \argmax{a, b} \log\left(\prod^{n}_{i}\exp{ -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}}\right) \notag \\
&= \argmax{a, b} \sum^{n}_{i} -\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2} \notag \\
&= \argmin{a, b} \sum^{n}_{i}\frac{\left(y_i - f(x_i, a, b)\right)^2}{2\sigma_i^2}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Thus, a probability maximization turns into the minimization of a function, which is defined as the **objective function**. By defining another term, called **residual**, denoted as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
z = \bar{y} - f(\bar{x}, a, b)
\label{eq:residual}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\bar{x}@f$ and @f$\bar{y}@f$ are both observed data, the minimization can then be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
\hat{a}, \hat{b} = \argmin{a, b} \sum^{n}_{i}\frac{z(x_i, y_i, a, b)^2}{2\sigma_i^2}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

In general cases where @f$m@f$ parameters need to be optimized, denoted as @f$\mathbf{p} = (p_0, p_1, \ldots, p_m)@f$, the _objective function_ is expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
\mathcal{F}(\mathbf{p}) = \sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$f(x_i, \mathbf{p})@f$ can be any functions, including non-linear ones in parameters @f$\mathcal{p}@f$.

### Minimization process

The minimization process in this project is using so-called _Gaussian Newton's Method_ \cite wiki:GaussNewton, which basically assumes the linear relations of the optimization parameters in the calibration equation:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
f(x, \mathbf{p}) = f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} + \nabla_{\mathbf{p}} f(x, \mathbf{p}) \bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \cdot(\mathbf{p} - \mathbf{p}_\text{init})
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Thus, the iteration update on the parameters @f$\mathbf{p}@f$, which is derived from the traditional _Newton's Method_, can be further simplified as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{flalign}{
    & & \nabla_{\mathbf{p}}^2 \mathcal{F}\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \, \delta \mathbf{p} &= \nabla_{\mathbf{p}} \mathcal{F}\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \notag & \\
    &\implies & \nabla_{\mathbf{p}}^2 \left(\sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2} \right)\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \, \delta\mathbf{p} &= \nabla_{\mathbf{p}} \left( \sum^{n}_{i}\frac{\left(y_i - f(x_i, \mathbf{p})\right)^2}{2\sigma_i^2} \right)\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \notag & \\
    &\implies & \left(\sum^{n}_i \frac{\nabla_{\mathbf{p}} f(x_i, \mathbf{p}) \nabla_{\mathbf{p}}^{\dagger} f(x_i, \mathbf{p})}{\sigma_i^2} \right)\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} \, \delta\mathbf{p} &= - \sum^{n}_i \frac{z(x_i, y_i, \mathbf{p}) \nabla_{\mathbf{p}} f(x_i, \mathbf{p})}{\sigma_i^2}\bigg\rvert_{\mathbf{p} = \mathbf{p}_\text{init}} &
    \label{eq:hessian}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$z(x_i, y_i, \mathbf{p})@f$ is defined from equation @f$\eqref{eq:residual}@f$. The left-hand side matrix, @f$\nabla_{\mathbf{p}}^2 \mathcal{F}@f$, is called _Hessian matrix_, which is simplified to be the external product of the gradient of the calibration function@f$f(x, \mathbf{p})@f$ in the Gaussian Newton's method. There are four prerequisites for a successful minimization:

1. Initial values of all parameters @f$\mathbf{p}@f$ must be available.
2. Error values @f$\sigma_i@f$ must not be zero.
3. Hessian matrix on the left-hand side must not have rank-deficit and must be invertible.
4. Hessian matrix on the left-hand side must be positive definite.

Fortunately, a majority of the calibration equations concerning real-world detectors already fulfilled these prerequisites, except the third one, which can be easily fixed by fixing some parameters (see section). Nevertheless, there are two more issues when applying this minimization to the real experimental data:

- Both @f$x@f$ and @f$y@f$ have error values. Ignoring @f$x@f$ error values could cause inaccuracies on the optimized parameter values.
- Number of parameters could grow very large when the calibration involves with data from lots of tracks. At some points, trying to invert the Hessian matrix becomes impractical and very expensive.

Thus, in the next sessions, several methods addressing these issues will be explained in detail.

## Millepede

Millepede algorithm is used for reducing the dimension of the Hessian matrix if the objective function has the following prerequisites:

@anchor list_pre_req

1. Parameters can be categorized into one set of parameter group @f$\mathbf{p}@f$, called **global parameters** and other sets of parameter groups @f$\mathbf{q}_i@f$, called **local parameters**.
2. Each data point @f$(x_n, y_n)@f$ is concerned with the global parameters and only _one_ local parameter group, i.e. the objective function for each data point must be expressed as @f$\mathcal{F}_n(x_n, y_n, \mathbf{p}, \mathbf{q}_i)@f$. Parameters from different local parameter are not mixed with each other.
3. Only the updates on global parameters are necessary.

### Calibration of detectors

Detector calibration is essential for accurate measurements and is usually done by determining so-called _calibration parameters_. Many detectors, especially tracking detectors, consist of multiple detector modules, in various of materials such as silicon, plastic (in case of scintillators), metal, crystal etc. These modules can also be in different shapes, such as strip, bar, pixel or plane. Despite all these varieties and completely different mechanics on measurements, they follow a very similar calibration procedures: by utilizing the tracks from external particles, calibration parameters can be determined via some pre-known properties of the tracks, such as being a straight line, having a certain curvature, or travelling at a certain speed.

One of popular choices of external particles is the cosmic muon, which is traveling at nearly the speed of light and highly penetrable, ensuring the straight tracks inside the detector. Other popular choices could be the radiation particles (gamma ray, electrons or alpha particles) from a radioactive source or protons from a particle accelerator. The choice of the external particles for the calibration is highly dependent on the detection efficiency provided by the detector module. These particles may not be the same particles during the actual experiment, but their availability and abundance can provide enough sample sizes needed during the parameter optimization.

Calibration parameters can characterize different factors. One of the common factors is the displacement or misalignments of the detector modules, which introduces some inaccuracies on position values. But they can also represent some other non-spacial factors like time delays or effective speed of light inside a scintillation bar. In a mathematical point of view, calibration parameters represent correlations among _observables_ of a detector module through equation:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    0 = f(\mathbf{x}; \mathbf{p})
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x} = (x_1, \ldots, x_n)@f$ is a set of observables from the detector module and @f$\mathbf{p} = (p_1, \ldots, p_m)@f$ are corresponding calibration parameters. These observables can be any quantifiable values of the detector, such as assumed position of the modules, amplitude or the time of the signal from readouts, etc.

#### Example

@anchor fig_track
<div align="center">
  <img src="track_detector.svg" alt="Diagram" width="600">
  <br>
  <em>Figure 1: Conceptual scheme of a particle track through a detector.</em>
</div>

For example, imagine a set of detector modules located along the x-axis and separated from each other at a certain distance (see @ref fig_track "figure 1"). Each module is capable of measuring y value by itself, but is displaced along the y direction with an unknown distance. Thus, the y value measured by each module is inaccurate and its relation to the correct y value of the track can be depicted by the equation:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    y_\text{track} = y_\text{meas} + y_\text{offset}
    \label{eq:trackeq1}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Here the observable is simple a value @f$y_\text{meas}@f$ and x position of the module while the calibration parameter is the @f$y_\text{offset}@f$ due to the displacement. On the other hand, knowing the track is a straight line, another relation between the real y value and the module x position can be given as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    y_\text{track} = a \cdot x_\text{meas} + b
    \label{eq:trackeq2}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$a@f$ and @f$b@f$ are the slope and offset of the particle track and @f$x_\text{meas}@f$ represents the x position of the module. Combining equations @f$\eqref{eq:trackeq1}@f$ and @f$\eqref{eq:trackeq2}@f$ gives the correlation equation between the observables and calibration parameters:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    0 = a \cdot x_\text{meas} + b - y_\text{meas} - y_\text{offset}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

#### Generalization

Equations @f$\eqref{eq:trackeq1}@f$ and @f$\eqref{eq:trackeq2}@f$ can be generalized into any equations with:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->

\f{flalign}{
& &\mathbf{y}_\text{track} &= \mathbf{f}(\mathbf{y}, \mathbf{p}) \notag &\\
& &\mathbf{y}_\text{track} &= \mathbf{g}(\mathbf{x}, \mathbf{q}) \notag &\\
&\implies & 0 &= \mathbf{g}(\mathbf{x}, \mathbf{q}) - \mathbf{f}(\mathbf{y}, \mathbf{p}) &
\label{eq:track_gen}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x}@f$ and @f$\mathbf{y}@f$ are all observables, @f$\mathbf{p}@f$ is the module related parameters and @f$\mathbf{q}@f$ are the track related parameters. It's easy to see that equation @f$\eqref{eq:track_gen}@f$ can be treated as the calibration equation seen from equation @f$\eqref{eq:cal}@f$ and undergo a similar optimization process as seen in linear regression. When using data from all modules and tracks, the calibration equation for @f$i@f$-th module and @f$j@f$-th track would be:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    0 = \mathbf{g}(\mathbf{x}^i, \mathbf{q}^j) - \mathbf{f}(\mathbf{y}^i, \mathbf{p})
    \label{eq:gen_cal}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Suppose the total number of tracks is @f$n_\text{track}@f$, the total number of module related parameter @f$\mathcal{p}@f$ is @f$n_p@f$, the total number of track related parameters for each track is @f$n_q@f$, the dimension of the Hessian matrix seen in equation @f$\eqref{eq:hessian}@f$ would be:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \texttt{dim} \ \mathcal{H} = n_p + n_\text{track} \cdot n_q
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The computational complexity of the inversion of a matrix using Choleskey decomposition is @f$\mathcal{O}(n^3)@f$ where @f$n@f$ is the dimension of the symmetric matrix. Most of the time, the number of parameters for each track @f$n_q@f$ and the total number of module parameters @f$n_p@f$ are fixed. Thus, if using the direct linear regression, the computational complexity would be @f$\mathcal{O}(n_\text{track}^3)@f$, which is impractical as the tracks used for the calibration can be in the order of millions.

Fortunately, calibration of the tracking detectors fulfills the @ref list_pre_req "prerequisites" of millepede algorithm:

1. All parameters can be categorized into global and local parameters, where global parameters contain the parameters related to modules and local parameters related to the tracks.
2. The equation @f$\eqref{eq:gen_cal}@f$ shows that local parameters from different tracks are not mixed.
3. Only the module-related parameter needs to be optimized as only they are used for the calculation of the track positions.

### Dimension reduction

The parameter update of the equation @f$\eqref{eq:hessian}@f$ can be adjusted with the introduction of local and global parameters:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation*}{
    \left(\sum^{m}_j \sum^{n}_i \frac{\nabla f(x_{ij}, \mathbf{p}, \mathbf{q}_j) \nabla^{\dagger} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2} \right) \, \delta(\mathbf{\hat{p}},\, \mathbf{\hat{q}}) = - \sum^{m}_j \sum^{n}_i \frac{z(x_{ij}, y_{ij}, \mathbf{p}, \mathbf{q}_j) \nabla f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The indices @f$i@f$ and @f$j@f$ represent the index of the data point in a track and index of each track respectively.
The first order derivative @f$\nabla@f$ applies to all parameters @f$(\mathbf{p},\, \mathbf{q})@f$, i.e. @f$(\mathbf{p}, \mathbf{q}_1, \ldots, \mathbf{q}_m)@f$. If @ref list_pre_req "prerequisites" are met, it's possible to transform this equation into another equation which only updates the global parameter, where the dimension of the factor matrix is only equal to the number of global parameters and independent of number of tracks. The derivation of this transformation is delayed to next section and the resulting equation can be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \mathcal{C} \cdot \mathbf{\hat{p}} = - \mathbf{g}
    \label{eq:milleEq}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

The _factor matrix_ @f$\mathcal{C}@f$ is a summation of two factor matrices:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \mathcal{C} := \sum^{m}_{j} \mathcal{C}^1_{j} + \sum^{m}_{j} \mathcal{C}^2_{j}
    \label{eq:factorMat}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

By defining the "Jacobian" operator as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->

\f{align}{
\mathbf{\Delta}_{\mathbf{p}} f(\mathbf{p}, \mathbf{q}) &:= \nabla_{\mathbf{p}} f(\mathbf{p}, \mathbf{q}) \nabla_{\mathbf{p}}^{\dagger} f(\mathbf{p}, \mathbf{q}) \notag \\
\mathbf{\Delta}_{\mathbf{p}\mathbf{q}} f(\mathbf{p}, \mathbf{q}) &:= \nabla_{\mathbf{p}} f(\mathbf{p}, \mathbf{q}) \nabla_{\mathbf{q}}^{\dagger} f(\mathbf{p}, \mathbf{q})
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

and defining the diagonal factor and off-diagonal factor matrix as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->

\f{align}{
\Gamma_j &:= \sum^{n}_{i} \frac{\Delta_{\mathbf{q}_j} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2}\notag \\
\mathbf{G}_j &:= \sum^{n}_{i} \frac{\Delta_{\mathbf{p}\mathbf{q}_j} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2}
\label{eq:gamma}
\f}
<!-- prettier-ignore-end -->
<!-- LTeX: enabled=true -->

both factor matrices are defined as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->

\f{align}{
\mathcal{C}^1_{j} &:= \sum^{n}_{i} \frac{\Delta_{\mathbf{p}} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2} \notag \\
\mathcal{C}^2_{j} &:= \mathbf{G}_j \Gamma^{-1}_j \mathbf{G}_j^{\dagger}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Similarly, the "right-hand side" vector @f$\mathbf{g}@f$ is also defined as the summation of two vectors:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \mathbf{g} := \sum^{m}_{j} \mathbf{g}^1_{j} + \sum^{m}_{j} \mathbf{g}^2_{j}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where the two vectors are defined as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->

\f{align}{
\mathbf{g}^1_{j} &:= \sum^{n}_i \frac{z(x_{ij}, y_{ij}, \mathbf{p}, \mathbf{q}_j) \nabla_{\mathbf{p}} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2} \notag \\
\mathbf{g}^2_{j} &:= - \mathbf{G}_j \Gamma^{-1}_j \sum^{n}_{i} \frac{z(x_{ij}, y_{ij}, \mathbf{p}, \mathbf{q}_j) \nabla_{\mathbf{q}_j} f(x_{ij}, \mathbf{p}, \mathbf{q}_j)}{\sigma_{ij}^2}
\f}

<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Finally, from equation @f$\eqref{eq:milleEq}@f$, the update on the global parameters can be determined by:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
     \mathbf{\hat{p}} = - \mathcal{C}^{-1} \mathbf{g}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

Throughout the whole process, it can be found that

1. There are only two matrices that need to be inverted: @f$\mathcal{C}@f$, defined in equation @f$\eqref{eq:factorMat}@f$, and @f$\Gamma_{j}@f$, defined in equation @f$\eqref{eq:gamma}@f$.
2. The dimensions of both matrices are independent of the number of tracks.
3. Both matrices are symmetric and positive-definite. Thus, Choleskey decomposition can still be applied here.

### Derivation

### Millepede on calibration relation

## Regression with both x and y errors

To introduce @f$x@f$ errors both in the objective function and minimization step, some changes are required when calculating the maximum probability in equation @f$\eqref{eq:MLE}@f$. Assuming both @f$x@f$ and @f$y@f$ values follow the Gaussian distribution, and they are independent, the probability for the observed data points can be expressed as:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    Prob(\mathbf{x}, \mathbf{y}) \sim \prod^{n}_{i} \exp{ -\frac{\left(y_i - f(\mu_i, \mathbf{p})\right)^2}{2(\sigma^y_i)^2}} \exp{ -\frac{\left(x_i - \mu_i\right)^2}{2(\sigma^x_i)^2}}
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mu_i@f$ represents the true @f$x@f$ values and @f$f(\mu_i, \mathbf{p})@f$ the true @f$y@f$ values. Following a similar derivation, the objective function can then be expressed as

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{
    \mathcal{F}(\mathbf{p}, \boldsymbol{\mu}) = \sum^{n}_{i}\left(\frac{\left(y_i - f(\mu_i, \mathbf{p})\right)^2}{2(\sigma^y_i)^2} + \frac{\left(x_i - \mu_i\right)^2}{2(\sigma^x_i)^2} \right)
\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

It can be seen here that the @f$x@f$ true values @f$\boldsymbol{\mu}@f$ are treated as additional optimization parameters and its form is exactly the same as the objective function derived in the _Orthogonal Distance Regression_ (ODR)\cite boggs1989orthogonal.
