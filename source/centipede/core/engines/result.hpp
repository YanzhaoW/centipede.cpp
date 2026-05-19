#pragma once

#include "centipede/util/error_types.hpp"
#include <cstddef>
#include <cstdint>
#include <format>
#include <utility>
#include <vector>

namespace centipede::core::engine
{
    /**
     * @brief Data structure for results.
     */
    template <typename DataType>
    struct Result
    {
        using IdxValuePair = std::pair<std::size_t, DataType>;
        ErrorCode error_status = ErrorCode::invalid;          //!< Error enum if existed.
        std::size_t rank_deficit = 0;                         //!< Rank deficit value.
        uint64_t n_entries = 0;                               //!< Total number of entries read.
        uint64_t n_entries_rejected = 0;                      //!< Total number of entries rejected.
        std::vector<DataType> eigen_values;                   //!< Eigen values of global factor matrix.
        std::vector<std::size_t> redundant_parameter_indices; //!< Indices of parameters that are linear dependent.
        std::vector<IdxValuePair> parameters;                 //!< Resulting parameter values.
    };

} // namespace centipede::core::engine

namespace centipede
{
    /**
     * @brief Type alias for better user interface.
     */
    template <typename DataType>
    using Result = core::engine::Result<DataType>;
} // namespace centipede

/**
 * @brief Formating of the object with type centipede::Result
 */

template <typename DataType>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::Result<DataType>>
{
    using Result = centipede::Result<DataType>;
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    static constexpr auto format(const Result& result, std::format_context& ctx)
    {
        const auto percentage = (result.n_entries == 0) ? 0.
                                                        : static_cast<double>(result.n_entries_rejected) /
                                                              static_cast<double>(result.n_entries) * 100.;
        if (result.error_status == centipede::ErrorCode::success)
        {
            return std::format_to(ctx.out(),
                                  "Total entries: {}\t Rejected entries: {}\t Rejected rate: {:.2}%",
                                  result.n_entries,
                                  result.n_entries_rejected,
                                  percentage);
        }
        else
        {

            return std::format_to(ctx.out(),
                                  "Error: {}\n"
                                  "Rank deficit: {}. Possible redundant parameter indices: {}\n"
                                  "Total entries: {}\t Rejected entries: {}\t Rejected rate: {:.2}%\n"
                                  "Eigen values from global factor matrix: {}",
                                  result.error_status,
                                  result.rank_deficit,
                                  result.redundant_parameter_indices,
                                  result.n_entries,
                                  result.n_entries_rejected,
                                  percentage,
                                  result.eigen_values);
        }
    }
};
