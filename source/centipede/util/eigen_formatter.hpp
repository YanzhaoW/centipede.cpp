#pragma once
#include <Eigen/Core>
#include <concepts>
#include <format>
#include <sstream>

/**
 * @brief Formatting for eigen matrices
 */
template <typename EigenMatType>
    requires std::derived_from<EigenMatType, Eigen::EigenBase<EigenMatType>>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<EigenMatType>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    static constexpr auto format(const EigenMatType& eigen_mat, std::format_context& ctx)
    {
        auto sstream = std::stringstream{};
        sstream << eigen_mat;
        return std::format_to(ctx.out(), "{}", sstream.str());
    }
};
