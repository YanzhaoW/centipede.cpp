#pragma once

/**
 * @brief Common type traits for the project
 */

#include <type_traits>
#include <utility>

namespace centipede::internal
{
    /**
     * @brief Not a type of std::pair
     * @tparam T Any type
     */
    template <typename T>
    struct IsPair : public std::false_type
    {
    };

    /**
     * @brief A type of std::pair
     * @tparam U First type of std::pair
     * @tparam T Second type of std::pair
     */
    template <typename U, typename T>
    struct IsPair<std::pair<U, T>> : public std::true_type
    {
        using first_type = U;
        using second_type = T;
    };

} // namespace centipede::internal
