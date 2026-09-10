#pragma once

#include <format>

namespace centipede
{

    template <typename T, typename Formatter>
    struct FormatHelper
    {
        const T* data_ptr = nullptr;
        const Formatter* formatter_ptr = nullptr;
        FormatHelper(const T& data, const Formatter& formatter)
            : data_ptr{ &data }
            , formatter_ptr{ &formatter }
        {
        }
    };
} // namespace centipede

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
