#pragma once

#include "centipede/core/config.hpp"
#include "centipede/core/engines/base_engine.hpp"
#include "centipede/core/engines/eigen_engine.hpp" // IWYU pragma: keep
#include "centipede/core/engines/engine_concept.hpp"
#include "centipede/core/engines/engine_log.hpp"
#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/par_id_map.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <algorithm>
#include <assert.hpp>
#include <cstddef>
#include <expected>
#include <functional>
#include <ranges>
#include <unordered_map>
#include <utility>

#ifdef HAS_LIBASSERT
#include "libassert/assert.hpp"
#else
#include <cassert>
#endif

namespace centipede::core::engine
{
    /**
     * @brief Master interface class.
     */
    template <typename DataType, MasterOpt opt = {}>
        requires EngineLike<opt.engine_type, DataType>
    class Master
    {
      public:
        /**
         * @brief Temporary state variables.
         */
        struct State
        {
            std::size_t next_point_index = 0; //!< Index used for each new entrypoint added.
            Entry<DataType> entry;            //!< Data storing the current entry.
        };

        ~Master() = default;
        Master(const Master& other) = delete;
        Master(const Master&& other) = delete;
        auto operator()(const Master& other) -> Master& = delete;
        auto operator()(const Master&& other) -> Master& = delete;

        using ResultType = Result<DataType>;
        using EngineImp = Engine<opt.engine_type, DataType>;
        using DataTypeUsed = DataType;
        using Conf = Config<DataType>;

        explicit Master(const Conf& config)
            : config_{ config }
            , par_id_map_{ config_.n_globals, config_.fixed_parameter_ids }
            , slave_engine_{ config_ }
        {
            result_.parameters.reserve(config_.n_globals);
            config_.global_init_values = std::views::iota(0UZ, config_.n_globals) |
                                         std::views::transform([](auto idx) { return std::pair{ idx, DataType{} }; }) |
                                         std::ranges::to<std::unordered_map<std::size_t, DataType>>();
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
                    [this, &entry_point]() -> auto
                    {
                        current_state_.entry.n_locals = entry_point.get_n_locals();
                        add_measurement(entry_point);
                        add_locals(entry_point);
                        add_globals(entry_point);
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
            return slave_engine_.fill_data(current_state_.entry, par_id_map_)
                .and_then(
                    [this] -> auto
                    {
                        auto res = slave_engine_.analyze(config_.alpha);
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
        auto solve() -> VoidError { return solve(result_); }

        auto solve(ResultType& result) -> VoidError
        {
            slave_engine_.add_to_globals(globals_);
            slave_engine_.add_to_result(result);
            EngineImp::solve(globals_, result, par_id_map_);

            log_ += slave_engine_.get_log();

            return (result.error_status == ErrorCode::success) ? VoidError{} : std::unexpected{ result.error_status };
        }

        auto set_global_init_value(std::size_t global_par_idx, DataType val) -> VoidStr
        {
            if (global_par_idx >= config_.n_globals)
            {
                return std::unexpected{ "Global parameter index (0-based)" };
            }
#ifdef HAS_LIBASSERT
            debug_assert(config_.fixed_parameter_ids.contains(global_par_idx));
#else
            assert(config_.fixed_parameter_ids.contains(global_par_idx));
#endif

            config_.global_init_values.at(global_par_idx) = val;
            return {};
        }

        auto set_global_init_values(const auto& global_init_values) -> VoidStr
        {

            for (auto& [key, value] : config_.global_init_values)
            {
                auto value_iter = global_init_values.find(key);

                if (value_iter == global_init_values.end())
                {
                    continue;
                }

                value = value_iter->second;
            }
            return {};
        }

        [[nodiscard]] auto get_current_state() const -> const State& { return current_state_; }

        [[nodiscard]] auto get_engine() const -> const auto& { return slave_engine_; }

        [[nodiscard]] auto get_result() const -> const auto& { return result_; }

        [[nodiscard]] auto get_par_id_map() const -> const auto& { return par_id_map_; }

        [[nodiscard]] auto get_log() const -> const auto& { return log_; }

        [[nodiscard]] auto get_config() const -> const auto& { return config_; }

      private:
        Conf config_;
        ParIdMap par_id_map_;
        ResultType result_;
        State current_state_;
        EngineImp slave_engine_{};
        Log log_{};

        // TODO: represent globals as mdspan, instead of relying on eigen.
        EngineImp::Globals globals_{};

        void reset_state()
        {
            current_state_.next_point_index = 0;
            current_state_.entry.global_derivs.clear();
            current_state_.entry.local_derivs.clear();
            current_state_.entry.measurements.clear();
            current_state_.entry.n_locals.reset();
        }

        auto check_entrypoint_valid(const auto& entry_point) -> VoidError
        {
            const auto n_locals = entry_point.get_n_locals();
            if (current_state_.entry.n_locals.has_value() and current_state_.entry.n_locals.value() != n_locals)
            {
                return std::unexpected{ ErrorCode::handler_incomp_n_locals };
            }
            const auto n_globals = config_.n_globals;
            if (std::ranges::any_of(entry_point.get_globals(),
                                    [n_globals](const auto& idx_value) -> bool
                                    { return idx_value.first >= n_globals; }))
            {
                return std::unexpected{ ErrorCode::analysis_global_idx_too_large };
            }
            return {};
        }

        auto add_measurement(const auto& entry_point)
        {
            auto initial_value_offset = std::ranges::fold_left(
                entry_point.get_globals() |
                    std::views::transform(
                        [this](const auto& idx_value) -> DataType
                        { return idx_value.second.value * config_.global_init_values.at(idx_value.first); }),
                DataType{},
                std::plus{});
            current_state_.entry.measurements.emplace_back(entry_point.get_measurement().value - initial_value_offset,
                                                           entry_point.get_measurement().error);
        }

        auto add_locals(const auto& entry_point)
        {
            std::ranges::copy(
                std::views::zip_transform(
                    [this](auto local_idx, auto deriv) -> Entry<DataType>::Deriv
                    { return std::pair{ current_state_.next_point_index, std::pair{ local_idx, deriv } }; },
                    std::views::iota(0),
                    entry_point.get_locals()),
                std::back_inserter(current_state_.entry.local_derivs));
        }

        auto add_globals(const auto& entry_point)
        {
            std::ranges::copy(
                entry_point.get_globals() |
                    std::views::transform([this](const auto& deriv) -> Entry<DataType>::Deriv
                                          { return std::pair{ current_state_.next_point_index, deriv }; }),
                std::back_inserter(current_state_.entry.global_derivs));
            std::ranges::sort(current_state_.entry.global_derivs,
                              [](const Entry<DataType>::Deriv& left, const Entry<DataType>::Deriv& right) -> bool
                              {
                                  return left.first < right.first ||
                                         ((left.first == right.first) && (left.second.first < right.second.first));
                              });
        }
    };

} // namespace centipede::core::engine
