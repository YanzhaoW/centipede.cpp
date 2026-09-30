#pragma once

/**
 * @brief Types for returns
 */

#include "centipede/util/error_code.hpp"
#include <expected>

namespace centipede
{

    /**
     * @brief Template alias for expected return values.
     */
    template <typename T = void>
    using EnumError = std::expected<T, ErrorCode>;

    /**
     * @brief Template alias for an expected void result or an enum class error.
     */
    using VoidError = std::expected<void, ErrorCode>;
} // namespace centipede
