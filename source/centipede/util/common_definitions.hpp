#pragma once

#include <cstddef>

namespace centipede::common
{
    constexpr auto DEFAULT_BUFFER_SIZE =
        std::size_t{ 10000 }; //!< Default maximum buffer size for binary readers/writers.

    constexpr auto EIGEN_APPROX_PRECISION =
        0.001; //!< Precision to compare whether two floating point values are equal. See Eigen::DenseBase::isApprox().
    constexpr auto significance_level_3_sigma = 0.0027;
    constexpr auto significance_level_5_sigma = 5.7e-7;

} // namespace centipede::common
