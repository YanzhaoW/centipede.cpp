#pragma once

#include "centipede/util/common_definitions.hpp"
#include <cstddef>
#include <set>

namespace centipede
{
    /**
     * @brief Configurations for centipede
     */
    template <typename DataType>
    struct Config
    {
        DataType chi2_factor = 50.; //!< Down scaling of the chi2 value filter.
        std::size_t n_globals = 0;  //!< Number of global parameters.
        // NOLINTBEGIN (readability-redundant-member-init)
        std::set<std::size_t> fixed_parameter_ids = {}; //!< IDs of the fixed global parameters.
        // NOLINTEND (readability-redundant-member-init)
        DataType alpha = static_cast<DataType>(
            common::significance_level_3_sigma); //!< Significance level to reject the current entry data.
    };

} // namespace centipede
