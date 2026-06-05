#pragma once

#include <cstdint>
#include <format>

namespace centipede
{
    /**
     * @brief Enumerations for the centipede program
     */
    enum class ErrorCode : uint8_t
    {
        // TODO: duplication of comments
        invalid,                    //!< Error due to no evaluation!
        success,                    //!< No error. All good!
        handler_incomp_n_locals,    //!< Incompatible number of local variables from the current entrypoint.
        writer_neg_or_zero_sigma,   //!< Zero or negative sigma occurs. See @ref writer::Binary.
        writer_buffer_overflow,     //!< Buffer size is too small for a new entry occurs. See @ref writer::Binary.
        writer_entrypoint_rejected, //!< Entrypoint is rejected due to absence of non-zero derivs. See @ref
                                    //!< writer::Binary.
        writer_file_fail_to_open,   //!< File failed to be open.
        writer_uninitialized,       //!< Write is not initialized.
        analysis_local_fit_rank_deficit,
        analysis_local_fit_low_stat,
        analysis_local_fit_rejected,
        analysis_rank_deficit, //!< Rank deficit occurred during the analysis.
        analysis_zero_global_factor_mat,
        analysis_global_negative_definite,
        analysis_global_idx_too_large,
        analysis_factor_matrix_zero, //!< Global factor matrix is zero matrix.
        analysis_empty_entry,
        analysis_rhs_vector_zero, //!< Global right-hand-side vector is zero vector.
        reader_file_fail_to_open, //!< Input file failed to be open.
        reader_file_fail_to_read, //!< Input file failed to read
        reader_uninitialized,     //!< Reader is not initialized.
        reader_buffer_overflow,   //!< Buffer size is too small for a new entry occurs. See @ref reader::Binary.
        reader_invalid_filename,  //!< Filename is invalid or empty
    };

} // namespace centipede

/**
 * @brief Formatter for @ref centipede::ErrorCode "ErrorCode"
 *
 */
template <>
// NOLINTNEXTLINE (bugprone-std-namespace-modification)
struct std::formatter<centipede::ErrorCode>
{
    static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    static constexpr auto format(const centipede::ErrorCode& error_code, std::format_context& ctx)
    {
        using enum centipede::ErrorCode;
        switch (error_code)
        {
            // TODO: the error messages here are duplicated from the comments in the enum class. Maybe there is a better
            // way to put them together?
            case success:
                return std::format_to(ctx.out(), "No error. All good!");
            case handler_incomp_n_locals:
                return std::format_to(ctx.out(),
                                      "Handler: Incompatible number of local variables from the current entrypoint.");
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
            case analysis_local_fit_rank_deficit:
                return std::format_to(ctx.out(), "Rank deficit occurred during the local fitting.");
            case analysis_local_fit_low_stat:
                return std::format_to(ctx.out(),
                                      "Sample size too small to calculate the chi-square value for the local fit.");
            case analysis_local_fit_rejected:
                return std::format_to(ctx.out(),
                                      "p-value from the local fitting is smaller than the significance level. Current "
                                      "entry is rejected.");
            case analysis_rank_deficit:
                return std::format_to(ctx.out(), "Rank deficit occurred during the analysis.");
            case analysis_zero_global_factor_mat:
                return std::format_to(ctx.out(), "Global factor matrix is zero.");
            case analysis_global_idx_too_large:
                return std::format_to(
                    ctx.out(), "Global index (0-based) is larger than or equal to the size of global parameters.");
            case analysis_factor_matrix_zero:
                return std::format_to(ctx.out(), "Global factor matrix is zero matrix.");
            case analysis_rhs_vector_zero:
                return std::format_to(ctx.out(), "Global right-hand-side vector is zero vector.");
            case analysis_empty_entry:
                return std::format_to(ctx.out(), "Current entry has no entry points.");
            case reader_file_fail_to_open:
                return std::format_to(ctx.out(), "Reader: Failed to open the file.");
            case reader_uninitialized:
                return std::format_to(ctx.out(), "Reader: Must be initialized beforehand!");
            case reader_file_fail_to_read:
                return std::format_to(ctx.out(), "Reader: Failed to read the file.");
            case reader_buffer_overflow:
                return std::format_to(ctx.out(), "Reader: Cannot read the file. Buffer size will be exceeded!");
            case reader_invalid_filename:
                return std::format_to(ctx.out(), "Reader: Filename is either empty or invalid!");
            case invalid:
                return std::format_to(ctx.out(), "Error due to no evaluation!");
            default:
                break;
        }
        return std::format_to(ctx.out(), "invalid error code");
    }
};
