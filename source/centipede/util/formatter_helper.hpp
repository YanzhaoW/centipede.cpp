#pragma once

#include <format>

namespace centipede
{

    /**
     * @brief Helper function for passing the parser options for the floating-point values.
     */
    template <typename T, typename Formatter>
    struct FormatHelper
    {
        const T* data_ptr = nullptr;
        const Formatter* formatter_ptr = nullptr;

        /**
         * @brief Constructor
         *
         * @param data value to be formatted
         * @param formatter Formatter object.
         */
        FormatHelper(const T& data, const Formatter& formatter)
            : data_ptr{ &data }
            , formatter_ptr{ &formatter }
        {
        }
    };
} // namespace centipede

/**
 * @brief Formatter for format helper
 */
template <typename T, typename Formatter>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::FormatHelper<T, Formatter>>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static auto format(centipede::FormatHelper<T, Formatter> formatter, std::format_context& ctx)
    {
        return formatter.formatter_ptr->format(*(formatter.data_ptr), ctx);
    }
};
