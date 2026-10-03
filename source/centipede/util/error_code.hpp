#pragma once

#include "centipede/util/error_types.hpp"
#include <expected>
#include <format>
#include <string>
#include <utility>

namespace centipede
{
    /**
     * @brief Error code class for error identification
     */
    class ErrorCode
    {
      public:
        /**
         * @brief Constructor with error type and custom message
         * @param error_type Error type enum
         * @param msg Error message
         */
        constexpr ErrorCode(ErrorType error_type, std::string msg)
            : error_type_{ error_type }
            , message_{ std::move(msg) }
        {
        }

        /**
         * @brief Constructor only with error type
         * @param error_type Error type enum
         */
        constexpr explicit ErrorCode(ErrorType error_type)
            : ErrorCode{ error_type, "" }
        {
        }

        /**
         * @brief Default constructor
         */
        constexpr ErrorCode()
            : ErrorCode{ ErrorType::success }
        {
        }

        auto operator=(ErrorType error_type) -> ErrorCode&
        {
            error_type_ = error_type;
            return *this;
        }

        static auto Error(ErrorType error_type, std::string msg = "")
        {
            return std::unexpected{ ErrorCode{ error_type, std::move(msg) } };
        }
        static auto Error(const ErrorCode& error_code) { return std::unexpected{ error_code }; }

        static auto Error(std::string msg) { return std::unexpected{ ErrorCode{ ErrorType::any, std::move(msg) } }; }

        /**
         * @brief Return the stored error message
         *
         * If the error code is set without error message, the empty string would be returned.
         */
        [[nodiscard]] constexpr auto message() const -> const std::string& { return message_; };

        /**
         * @brief Return a message containing both the error type and error message
         *
         * If the error code is set without error message, the error message would be the format string of the error
         * type enum.
         *
         * @see @ref std::formatter<centipede::ErrorType>
         */
        [[nodiscard]] constexpr auto what() const -> std::string
        {
            if (message_.empty())
            {
                return std::format("Error type {:?}: {}", convert_error_type_to_str(error_type_), error_type_);
            }
            return std::format("Error type {:?}: {}", convert_error_type_to_str(error_type_), message_);
        };

        /**
         * @brief Return the error type enum stored in this error code
         */
        [[nodiscard]] constexpr auto error() const -> ErrorType { return error_type_; }

        /**
         * @brief Implicit conversion to boolean
         */
        constexpr explicit operator bool() const { return error_type_ == ErrorType::success; }

        /**
         * @brief Unitary comparison with a error type enum
         *
         * Equal when the stored error types are equal.
         */
        constexpr auto operator==(const ErrorType& other) const -> bool { return error_type_ == other; };

        /**
         * @brief Binary comparison with a error type enum
         *
         * Equal when the stored error types are equal.
         */
        friend constexpr auto operator==(const ErrorType& left, const ErrorCode& right) -> bool
        {
            return right == left;
        };

        /**
         * @brief Comparison with other error code.
         *
         * Equal when the stored error types are equal.
         */
        constexpr auto operator==(const ErrorCode& other) const -> bool { return error_type_ == other.error_type_; };

      private:
        ErrorType error_type_ = ErrorType::success;
        std::string message_;
    };
} // namespace centipede

/**
 * @brief Formatter for @ref centipede::ErrorCode "ErrorCode"
 */
template <>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::ErrorCode>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static constexpr auto format(const centipede::ErrorCode& error_code, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}", error_code.what());
    }
};
