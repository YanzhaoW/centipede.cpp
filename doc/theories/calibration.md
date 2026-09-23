## Calibration of detectors

Detector calibration is essential for accurate measurements and is usually done by determining so-called _calibration parameters_. Many detectors, especially tracking detectors, consist of multiple detector modules, in various of materials such as silicon, plastic (in case of scintillators), metal, crystal etc. These modules can also differ in shape, such as strip, bar, pixel or plane. Despite all these varieties and completely different mechanics of measurements, they follow a very similar calibration procedures: by utilizing the tracks from external particles, calibration parameters can be determined via some pre-known properties of the tracks, such as being a straight line, having a certain curvature, or travelling at a certain speed.

One of popular choices of external particles is the cosmic muon, which is traveling at nearly the speed of light and highly penetrable, ensuring the straight tracks inside the detector. Other popular choices could be the radiation particles (gamma ray, electrons or alpha particles) from a radioactive source or protons from a particle accelerator. The choice of the external particles for the calibration is highly dependent on the detection efficiency of the detector module. These particles may not be the same particles detected during the actual experiment, but their availability and abundance can provide enough sample sizes needed during the parameter optimization.

Calibration parameters can characterize different factors. One of the common factors is the displacement or misalignments of the detector modules, which introduces some inaccuracies on position values. But they can also represent some other non-spacial factors like time delays or effective speed of light inside a scintillation bar. In a mathematical point of view, calibration parameters represent correlations among _observables_ of a detector module through equation:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    0 = f(\mathbf{x}; \mathbf{p})

\f}
<!-- LTeX: enabled=true -->
<!-- prettier-ignore-end -->

where @f$\mathbf{x} = (x_1, \ldots, x_n)@f$ is a set of observables from the detector module and @f$\mathbf{p} = (p_1, \ldots, p_m)@f$ are corresponding calibration parameters. These observables can be any quantifiable values of the detector, such as assumed position of the modules, amplitude or the time of the signal from readouts, etc.

### Example

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

### Generalization

Equations @f$\eqref{eq:trackeq1}@f$ and @f$\eqref{eq:trackeq2}@f$ can be generalized with two arbitrary functions, @f$\mathbf{f}(\mathbf{y}, \mathbf{p})@f$, describing the track point from calibration parameters, and @f$\mathbf{g}(\mathbf{x}, \mathbf{q})@f$, describing the track point from a local track parameters:

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

where @f$\mathbf{x}@f$ and @f$\mathbf{y}@f$ are all observables, @f$\mathbf{p}@f$ is the module related parameters and @f$\mathbf{q}@f$ are the track related parameters. It's easy to see that equation @f$\eqref{eq:track_gen}@f$ can be treated as the calibration equation seen from equation @f$\eqref{eq:imp_func}@f$ and undergo a similar optimization process as seen in the regression for implicit equations. When using data from all modules and tracks, the calibration equation for @f$j@f$-th module and @f$k@f$-th track would be:

<!-- prettier-ignore-start -->
<!-- LTeX: enabled=false -->
\f{equation}{

    0 = \mathbf{g}(\mathbf{x}^{j,k}, \mathbf{q}^k) - \mathbf{f}(\mathbf{y}^{j,k}, \mathbf{p}^j)
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

However, calibration of the tracking detectors via external particles demonstrates the following properties:

1. All parameters can be categorized into global and local parameters, where global parameters contain the parameters related to modules and local parameters related to the tracks.
2. The equation @f$\eqref{eq:gen_cal}@f$ shows that local parameters from different tracks are not mixed.
3. Only the module-related parameter needs to be optimized as only they are used for the calculation of the track positions.

These special properties give rise to a special matrix dimension reduction in the Millepede algorithm.
