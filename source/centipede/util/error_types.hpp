#pragma once
/**
 * @brief Types for errors
 */

#include <cstdint>
#include <format>

namespace centipede
{
    /**
     * @brief Enumerations for the centipede program
     */
    enum class ErrorCode : uint8_t
    {
        invalid,                    //!< An invalid error.
        writer_neg_or_zero_sigma,   //!< Zero or negative sigma occurs. See @ref writer::Binary.
        writer_buffer_overflow,     //!< Buffer size is too small for a new entry occurs. See @ref writer::Binary.
        writer_entrypoint_rejected, //!< Entrypoint is rejected due to absence of non-zero derivs. See @ref
                                    //!< writer::Binary.
        writer_file_fail_to_open,   //!< File failed to be open.
        writer_uninitialized,       //!< Write is not initialized.
    };

} // namespace centipede

/**
 * @brief Formatter for ErrorCode
 *
 */
template <>
struct std::formatter<centipede::ErrorCode>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static constexpr auto format(const centipede::ErrorCode& error_code, std::format_context& ctx)
    {
        using enum centipede::ErrorCode;
        switch (error_code)
        {
            case writer_neg_or_zero_sigma:
                return std::format_to(ctx.out(), "Writer: Sigma value in the entry point is 0.F or negative!");
            case writer_buffer_overflow:
                return std::format_to(ctx.out(), "Writer: Cannot add the entry point. Buffer size will be exceeded!");
            case writer_entrypoint_rejected:
                return std::format_to(ctx.out(),
                                      "Writer: Entry point is rejected due to the derivative values are all zeros!");
            case writer_file_fail_to_open:
                return std::format_to(ctx.out(), "Writer: Failed to open the file.");
            case writer_uninitialized:
                return std::format_to(ctx.out(), "Writer: Must be initialized beforehand!");
            case invalid:
            default:
        }
        return std::format_to(ctx.out(), "invalid error code");
    }
};
