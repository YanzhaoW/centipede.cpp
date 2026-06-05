#pragma once

#include "centipede/core/engines/base_engine.hpp"
#include "centipede/core/engines/eigen_engine.hpp" // IWYU pragma: keep
#include "centipede/core/engines/engine_concept.hpp"
#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <algorithm>
#include <cstddef>
#include <expected>
#include <ranges>
#include <utility>

namespace centipede::core::engine
{

    constexpr auto significance_level_3_sigma = 0.0027;
    constexpr auto significance_level_5_sigma = 5.7e-7;

    /**
     * @brief Master interface class.
     */
    template <typename DataType, MasterOpt opt = {}>
        requires EngineLike<opt.engine_type, DataType>
    class Master
    {
      public:
        /**
         * @brief Configuration for the #Master class.
         *
         */
        struct Config
        {
            std::size_t n_globals = 0;                 //!< Number of global parameters.
            double alpha = significance_level_3_sigma; //!< Significance level to reject the current entry data.
        };

        /**
         * @brief Temporary state variables.
         */
        struct State
        {
            std::size_t next_point_index = 0; //!< Index used for each new entrypoint added.
            Entry<DataType> entry;            //!< Data storing the current entry.
        };

        using ResultType = Result<DataType>;
        using EngineImp = Engine<opt.engine_type, DataType>;
        using DataTypeUsed = DataType;

        explicit Master(Config config)
            : config_{ config }
            , engine_imp_{ config_.n_globals }
        {
            result_.parameters.reserve(config_.n_globals);
        }

        /**
         * @brief Fill the entrypoint to the current entry.
         *
         * The global derivative values are sorted in ascending order by global parameter indicies.
         * @param entry_point Current entrypoint to be filled.
         * @return An expected value. True when the filling is successful.
         * #centipede::ErrorCode::handler_incomp_n_locals if local parameter numbers are changed during the current
         * entry.
         */
        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> VoidError
        {
            return check_entrypoint_valid(entry_point)
                .transform(
                    [this, &entry_point]()
                    {
                        current_state_.entry.n_locals = entry_point.get_n_locals();

                        current_state_.entry.measurements.push_back(entry_point.get_measurement());
                        current_state_.entry.sigmas.push_back(entry_point.get_sigma());
                        std::ranges::copy(
                            std::views::zip_transform(
                                [this](auto local_idx, auto deriv) -> Entry<DataType>::Deriv
                                { return std::pair{ current_state_.next_point_index, std::pair{ local_idx, deriv } }; },
                                std::views::iota(0),
                                entry_point.get_locals()),
                            std::back_inserter(current_state_.entry.local_derivs));
                        std::ranges::copy(
                            entry_point.get_globals() |
                                std::views::transform([this](const auto& deriv) -> Entry<DataType>::Deriv
                                                      { return std::pair{ current_state_.next_point_index, deriv }; }),
                            std::back_inserter(current_state_.entry.global_derivs));
                        std::ranges::sort(
                            current_state_.entry.global_derivs,
                            [](const Entry<DataType>::Deriv& left, const Entry<DataType>::Deriv& right) -> bool
                            {
                                return left.first < right.first ||
                                       ((left.first == right.first) && (left.second.first < right.second.first));
                            });
                        ++current_state_.next_point_index;
                    });
        }

        /**
         * @brief Fitting the current entry data.
         *
         * This operation can be async. The state is also reset to the default one.
         * @see ref
         */
        auto analyze() -> VoidError
        {
            return engine_imp_.fill_data(current_state_.entry)
                .and_then(
                    [this]()
                    {
                        auto res = engine_imp_.analyze(config_.alpha);
                        reset_state();
                        return res;
                    });
        }

        /**
         * @brief Calculate the update of the global parameters.
         *
         * @return name description
         * @see ref
         */
        auto solve() -> VoidError
        {
            engine_imp_.add_to_globals(globals_);
            engine_imp_.add_to_result(result_);
            EngineImp::solve(globals_, result_);

            return (result_.error_status == ErrorCode::success) ? VoidError{} : std::unexpected{ result_.error_status };
        }

        [[nodiscard]] auto get_current_state() const -> const State& { return current_state_; }

        [[nodiscard]] auto get_engine() const -> const auto& { return engine_imp_; }

        [[nodiscard]] auto get_result() const -> const auto& { return result_; }

      private:
        Config config_;
        ResultType result_;
        State current_state_;
        EngineImp engine_imp_{};

        //TODO: represent globals as mdspan, instead of relying on eigen.
        EngineImp::Globals globals_{};

        void reset_state()
        {
            current_state_.next_point_index = 0;
            current_state_.entry.global_derivs.clear();
            current_state_.entry.local_derivs.clear();
            current_state_.entry.measurements.clear();
            current_state_.entry.sigmas.clear();
            current_state_.entry.n_locals.reset();
        }

        template <std::size_t NLocals, std::size_t NGlobals>
        auto check_entrypoint_valid(const EntryPoint<NLocals, NGlobals>& entry_point) -> VoidError
        {
            const auto n_locals = entry_point.get_n_locals();
            if (current_state_.entry.n_locals.has_value() and current_state_.entry.n_locals.value() != n_locals)
            {
                return std::unexpected{ ErrorCode::handler_incomp_n_locals };
            }
            const auto n_globals = config_.n_globals;
            if (std::ranges::any_of(entry_point.get_globals(),
                                    [n_globals](const auto& idx_value) { return idx_value.first >= n_globals; }))
            {
                return std::unexpected{ ErrorCode::analysis_global_idx_too_large };
            }
            return {};
        }
    };

} // namespace centipede::core::engine
