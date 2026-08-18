#pragma once

#include <cstdint>
#include <format>

namespace centipede::core::engine
{
    /**
     * @brief Logging data during the whole run.
     */
    struct Log
    {
        uint64_t n_entries_read = 0;               //!< Total number of entries read.
        uint64_t n_entries_success = 0;            //!< Total number of entries used for updating global parameters.
        uint64_t n_entries_low_stat = 0;           //!< Total number of unused entries because of low stat.
        uint64_t n_entries_local_rank_deficit = 0; //!< Total number of unused entries because of rank deficit.
        uint64_t n_entries_rejected =
            0; //!< Total number of unused entries because of p-value below the significance level.

        auto operator+=(const Log& other) -> Log&
        {
            n_entries_read += other.n_entries_read;
            n_entries_success += other.n_entries_success;
            n_entries_low_stat += other.n_entries_low_stat;
            n_entries_local_rank_deficit += other.n_entries_local_rank_deficit;
            n_entries_rejected += other.n_entries_rejected;
            return *this;
        };
    };
} // namespace centipede::core::engine

/**
 * @brief Formatting for engine log
 */
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
template <>
struct std::formatter<centipede::core::engine::Log>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    static constexpr auto format(const centipede::core::engine::Log& log, std::format_context& ctx)
    {
        return std::format_to(ctx.out(),
                              "n_entries_read: {}, "
                              "n_entries_success: {}, "
                              "n_entries_low_stat: {}, "
                              "n_entries_local_rank_deficit: {}, "
                              "n_entries_rejected: {}",
                              log.n_entries_read,
                              log.n_entries_success,
                              log.n_entries_low_stat,
                              log.n_entries_local_rank_deficit,
                              log.n_entries_rejected);
    }
};
