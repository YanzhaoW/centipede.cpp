#pragma once

/**
 * @brief Types for returns
 */

#include "centipede/util/error_types.hpp"
#include <expected>
#include <string>

namespace centipede
{
    using StrError = std::expected<void, std::string>;

    /**
     * @brief Template alias for expected return values.
     */
    template <typename T = void>
    using EnumError = std::expected<T, ErrorCode>;
} // namespace centipede
