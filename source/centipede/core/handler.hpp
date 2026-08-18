#pragma once

#include "centipede/core/config.hpp"
#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/master_engine.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/common_traits.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>

namespace centipede
{

    template <typename DataType, core::engine::MasterOpt opt>
    class Handler;

    template <typename DataType = double, core::engine::MasterOpt opt = {}>
    static auto create(const Config<DataType>& config = {}) -> std::expected<Handler<DataType, opt>, std::string>;

    /**
     * @brief Main frontend handler to the library
     *
     * This class should handle all inputs and configurations from users
     */
    template <typename DataType = double, core::engine::MasterOpt opt = {}>
    class Handler
    {
      public:
        /**
         * @class Config
         * @brief Runtime configuration for handler.
         */
        using MasterEngineType = core::engine::Master<DataType, opt>;
        using Conf = Config<DataType>;

        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> VoidError
        {
            return master_engine_->add_entrypoint(entry_point);
        }

        auto set_global_init_values(const internal::MapLike auto& global_init_values)
        {
            master_engine_->set_global_init_values(global_init_values);
        }

        auto set_global_init_value(std::size_t global_idx, DataType val) -> VoidStr
        {
            return master_engine_->set_global_init_value(global_idx, val);
        }

        auto analyze_current_entry() -> EnumError<std::size_t>
        {
            auto n_points = master_engine_->get_current_state().entry.measurements.size();

            if (n_points == 0)
            {
                return std::unexpected{ ErrorCode::analysis_empty_entry };
            }

            return master_engine_->analyze().transform([n_points]() -> std::size_t { return n_points; });
        };

        [[nodiscard]] auto solve() -> VoidError { return master_engine_->solve(); }

        [[nodiscard]] auto solve(Result<DataType>& result) -> VoidError { return master_engine_->solve(result); }

        [[nodiscard]] auto get_master_engine() const -> const auto& { return *master_engine_; }
        [[nodiscard]] auto get_current_state() const -> const auto& { return master_engine_->get_current_state(); }

        [[nodiscard]] auto get_slave_entry_state() const -> const auto&
        {
            return master_engine_->get_engine().get_current_state();
        }

        [[nodiscard]] auto get_slave_engine() const -> const auto& { return master_engine_->get_engine(); }

        [[nodiscard]] auto get_result() const -> const auto& { return master_engine_->get_result(); }

        [[nodiscard]] auto get_config() const -> const auto& { return master_engine_->get_config(); }

      private:
        std::unique_ptr<MasterEngineType> master_engine_;

        /**
         * @brief Constructor
         *
         * This is the default constructor
         */
        explicit Handler(const Conf& config = {})
            : master_engine_{ std::make_unique<MasterEngineType>(config) }
        {
        }

        static auto check_config(const Conf& config) -> std::optional<std::string>
        {
            if (config.fixed_parameter_ids.size() >= config.n_globals)
            {
                return std::format("Invalid configuration: the number of global parameters ({}) should be larger than "
                                   "the number of fixed global parameters ({})",
                                   config.n_globals,
                                   config.fixed_parameter_ids.size());
            }
            return {};
        }

        friend auto create<DataType, opt>(const Config<DataType>& config)
            -> std::expected<Handler<DataType, opt>, std::string>;
    };

    template <typename DataType, core::engine::MasterOpt opt>
    auto create(const Config<DataType>& config) -> std::expected<Handler<DataType, opt>, std::string>
    {
        auto has_error = Handler<DataType, opt>::check_config(config);
        if (has_error)
        {
            return std::unexpected{ has_error.value() };
        }
        return Handler{ config };
    }
} // namespace centipede
