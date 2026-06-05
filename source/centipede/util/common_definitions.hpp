#pragma once

#include <cstddef>

namespace centipede::common
{
    constexpr auto DEFAULT_BUFFER_SIZE =
        std::size_t{ 10000 }; //!< Default maximum buffer size for binary readers/writers.

    constexpr auto EIGEN_APPROX_PRECISION =
        0.001; //!< Precision to compare whether two floating point values are equal. See Eigen::DenseBase::isApprox().
} // namespace centipede::common
