#pragma once

#include "centipede/util/formatter_helper.hpp"
#include <concepts>
#include <format>
#include <type_traits>

namespace centipede
{
    /**
     * @brief Struct representing the value of a measurement and its error.
     */
    template <typename T>
    struct ValueError
    {
        T value{}; //!< Value
        T error{}; //!< Error

        /**
         * @brief Constructor using value and error.
         * @param val Value
         * @param err Error
         */
        explicit ValueError(T val, T err)
            : value{ val }
            , error{ err }
        {
        }

        /**
         * @brief Constructor with only the value. Error will be the default value from T.
         * @param val Value
         */
        explicit ValueError(T val)
            : ValueError(val, T{})
        {
        }

        /**
         * @brief Constructor using convertible value and error.
         * @param val Value
         * @param err Error
         */
        explicit ValueError(std::convertible_to<T> auto val, std::convertible_to<T> auto err)
            : value{ static_cast<T>(val) }
            , error{ static_cast<T>(err) }
        {
        }

        /**
         * @brief Constructor with only the convertible value. Error will be the default value from T.
         * @param val Value
         */
        explicit ValueError(std::convertible_to<T> auto val)
            : ValueError(static_cast<T>(val), T{})
        {
        }

        /**
         * @brief Default constructor.
         */
        ValueError()
            : ValueError(T{}, T{})
        {
        }

        auto operator=(std::convertible_to<T> auto val) -> const auto&
        {
            value = static_cast<T>(val);
            error = T{};
            return *this;
        }

        constexpr auto operator<=>(const ValueError<T>& other) const = default;
    };

    using ValueErrorF = ValueError<float>;
    using ValueErrorD = ValueError<double>;
    using ValueErrorI = ValueError<int>;

    namespace internal
    {
        template <typename T>
        struct IsValueError : std::false_type
        {
        };

        template <typename T>
        struct IsValueError<ValueError<T>> : std::true_type
        {
            using data_type = T;
        };

        template <typename T>
        constexpr auto IsValueErrorV = IsValueError<T>::value;

        template <typename T, typename U>
        concept ValueErrorConvertible = std::convertible_to<T, U> or requires(T value_error) {
            requires IsValueErrorV<T>;
            requires std::convertible_to<typename IsValueError<T>::data_type, U>;
        };

    } // namespace internal

} // namespace centipede

/**
 * @brief Formatter for ValueError
 */
template <typename T>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::ValueError<T>>
{
    std::formatter<T> parse_option;

    constexpr auto parse(std::format_parse_context& ctx) { return parse_option.parse(ctx); }

    constexpr auto format(const centipede::ValueError<T>& value_error, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(),
                              "{} +/- {}",
                              centipede::FormatHelper{ value_error.value, parse_option },
                              centipede::FormatHelper{ value_error.error, parse_option });
    }
};
