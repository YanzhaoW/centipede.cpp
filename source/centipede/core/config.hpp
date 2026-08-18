#pragma once

#include "centipede/util/common_definitions.hpp"
#include <cstddef>
#include <format>
#include <set>
#include <unordered_map>

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
        std::set<std::size_t> fixed_parameter_ids = {};                 //!< IDs of the fixed global parameters.
        std::unordered_map<std::size_t, DataType> global_init_values{}; //!< Init values of global parameters.
        // NOLINTEND (readability-redundant-member-init)
        DataType alpha = static_cast<DataType>(
            common::significance_level_3_sigma); //!< Significance level to reject the current entry data.
    };

} // namespace centipede

/**
 * @brief Custom formatter for centipede::Config
 */
template <typename T>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::Config<T>>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    static constexpr auto format(const centipede::Config<T>& config, std::format_context& ctx)
    {
        return std::format_to(ctx.out(),
                              "chi2 factor: {}\n"
                              "n_globals: {}\n"
                              "fixed parameter ID list: {}\n"
                              "alpha: {}",
                              config.chi2_factor,
                              config.n_globals,
                              config.fixed_parameter_ids,
                              config.alpha);
    }
};
