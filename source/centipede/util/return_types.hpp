#pragma once

/**
 * @brief Types for returns
 */

#include "centipede/util/error_types.hpp"
#include <expected>
#include <string>

namespace centipede
{
    /**
     * @brief Template alias for expected string results.
     */
    template <typename T>
    using StrError = std::expected<T, std::string>;

    /**
     * @brief Template alias for expected return values.
     */
    template <typename T = void>
    using EnumError = std::expected<T, ErrorCode>;

    /**
     * @brief Template alias for an expected void result or an enum class error.
     */
    using VoidError = std::expected<void, ErrorCode>;

    /**
     * @brief Template alias for an expected void result or a string error.
     */
    using VoidStr = std::expected<void, std::string>;
} // namespace centipede
