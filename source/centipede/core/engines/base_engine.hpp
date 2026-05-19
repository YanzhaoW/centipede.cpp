#pragma once

#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <gsl/gsl_cdf.h>
#include <utility>

namespace centipede::core::engine
{
    /**
     * @brief Base engine classes.
     *
     */
    template <typename DataType>
    class Base
    {
      public:
        /**
         * @brief Structure to store state variables relating to the analysis of the current entry.
         */
        struct State
        {
            bool is_rejected = false;  //!< Flag showing whether current entry is rejected.
            std::size_t n_globals = 0; //!< Number of global parameters.
            std::size_t n_locals = 0;  //!< Number of the local parameters in the current entry.
            std::size_t n_points = 0;  //!< Number of entrypoints in the current entry.
            std::size_t ndf = 0;       //!< Current degree of freedom for the local fitting.
            double chi2 = 0.;          //!< Current \f$\chi^2\f$ square value for the local fitting.
            double p_value = 0.;       //!< Current p_value for the local fitting.
        };

        /**
         * @brief Logging data during the whole run.
         */
        struct Log
        {
            uint64_t n_entries_read = 0;               //!< Total number of entries read.
            uint64_t n_entries_success = 0;            //!< Total number of entries used for updating global parameters.
            uint64_t n_entries_low_stat = 0;           //!< Total number of unused entries because of low stat.
            uint64_t n_entries_local_rank_deficit = 0; //!< Total number of unused entries because of rank deficit.
            uint64_t n_entries_rejected =
                0; //!< Total number of unused entries because of p-value below the significance level.
        };

        /**
         * @brief Fill the entry data from #Master and increment the current entry counter.
         *
         * Before filling the entry data, all buffers in derived classes can be resized by defining the method
         * `resize_buffers()`. Values in the derived class can be set by defining following methods:
         * - `fill_measurements(const std::vector<DataType>&)`: Fill the value of measurements.
         * - `fill_sigmas(const std::vector<DataType>&)`: Fill the value of sigmas.
         * - `fill_local_derivs(const std::vector<Entry<DataType>::Deriv>&)`: Fill the value of the first-order local
         * derivatives.
         * - `fill_global_derivs(const std::vector<Entry<DataType>::Deriv>&)`: Fill the value of the first-order global
         * derivatives.
         *
         * Once the filling is completed, the entry count in the object is incremented by 1.
         *
         * @param self Reference to the caller object.
         * @param entry Entry data.
         * @see #Entry
         */
        void fill_data(this auto&& self, const Entry<DataType>& entry);

        /**
         * @brief Analyze the data from the current entry.
         *
         * The analysis process contains the following steps in order:
         *
         * 1. Local parameter fitting. Return the error immediately if any occurs.
         * 2. Calculate chi-square value and p-value from the local fitting. Return the error immediately if any occurs.
         * 3. Update factor matrix and right-hand-side (rhs) vector if the p-value is larger than the significance
         * level. Otherwise, the current entry is rejected, incrementing the rejected entry counter.
         *
         * @param self Reference to the caller object.
         * @param alpha Significance level to reject the current entry.
         * @return An error value if error occurs.
         */
        auto analyze(this auto&& self, double alpha) -> EnumError<>;

        [[nodiscard]] auto get_current_state() const -> const auto& { return state_; }
        [[nodiscard]] auto get_log() const -> const auto& { return log_; }

        /**
         * @brief Add number of total entries and rejected entries to the result.
         *
         * @param result description
         */
        void add_to_result(Result<DataType>& result)
        {
            result.n_entries += log_.n_entries_read;
            result.n_entries_rejected += log_.n_entries_rejected;
        }

      protected:
        explicit Base(std::size_t n_globals) { state_.n_globals = n_globals; }

      private:
        State state_;
        Log log_;
    };

    template <typename DataType>
    void Base<DataType>::fill_data(this auto&& self, const Entry<DataType>& entry)
    {
        if (not entry.n_locals)
        {
            return;
        }
        self.state_.n_points = entry.measurements.size();
        assert(self.state_.n_points == entry.sigmas.size());
        self.state_.n_locals = entry.n_locals.value();

        self.resize_buffers();

        self.fill_measurements(entry.measurements);
        self.fill_sigmas(entry.sigmas);
        self.fill_local_derivs(entry.local_derivs);
        self.fill_global_derivs(entry.global_derivs);

        ++self.log_.n_entries_read;
    }

    template <typename DataType>
    auto Base<DataType>::analyze(this auto&& self, double alpha) -> EnumError<>
    {
        return self.fit_local_pars()
            .and_then([&self]() -> EnumError<std::pair<std::size_t, double>>
                      { return self.calculate_local_fit_chi_square(); })
            .and_then(
                [&self, alpha](const auto& ndf_chi2) -> EnumError<>
                {
                    const auto p_value = gsl_cdf_chisq_Q(ndf_chi2.second, static_cast<double>(ndf_chi2.first));

                    self.state_.p_value = p_value;
                    self.state_.chi2 = ndf_chi2.second;
                    self.state_.ndf = ndf_chi2.first;

                    if (p_value > alpha) // Do not upgrade if p_value is too small
                    {
                        self.state_.is_rejected = false;
                        self.update_global_factor_matrix();
                        self.update_global_rhs_vector();
                        ++self.log_.n_entries_success;
                        return {};
                    }
                    self.state_.is_rejected = true;
                    ++self.log_.n_entries_rejected;
                    return std::unexpected{ ErrorCode::analysis_local_fit_rejected };
                })
            .transform_error(
                [&self](ErrorCode err)
                {
                    switch (err)
                    {
                        case ErrorCode::analysis_local_fit_rank_deficit:
                            ++self.log_.n_entries_local_rank_deficit;
                            break;
                        case ErrorCode::analysis_local_fit_low_stat:
                            ++self.log_.n_entries_low_stat;
                            break;
                        default:
                            break;
                    }
                    return err;
                });

        // TODO: Do something with bad fit
    }

    /**
     * @brief Empty base engine class. The real implementation is defined in its specialization.
     */
    template <MatrixEngine engine_type, typename DataType>
    class Engine
    {
    };

} // namespace centipede::core::engine
