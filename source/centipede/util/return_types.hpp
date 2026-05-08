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
    using StrError = std::expected<void, std::string>;

    /**
     * @brief Template alias for expected return values.
     */
    template <typename T = void>
    using EnumError = std::expected<T, ErrorCode>;

    /**
     * @brief Template alias for expected void results.
     */
    using VoidError = std::expected<void, ErrorCode>;
} // namespace centipede
