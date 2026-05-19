#pragma once

#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/master_engine.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>

namespace centipede
{

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
        struct Config
        {
            std::size_t n_globals = 0; //!< Number of global parameters.
        };
        using EngineType = core::engine::Master<DataType, opt>;

        /**
         * @brief Constructor
         *
         * This is the default constructor
         */
        explicit Handler(Config config = {})
            : config_{ config }
            , engine_{ typename EngineType::Config{ .n_globals = config.n_globals } }
        {
        }

        // /**
        //  * @brief Initialization of the instance
        //  *
        //  */
        // [[nodiscard]] auto init() -> EnumError<> {
        //     engine_.init();
        //     return {}; }

        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>
        {
            return engine_.add_entrypoint(entry_point);
        }

        auto analyze_current_entry() -> EnumError<std::size_t>
        {
            auto n_points = engine_.get_current_state().entry.measurements.size();

            return engine_.analyze().transform([n_points]() { return n_points; });
        };

        [[nodiscard]] auto get_current_state() const -> const auto& { return engine_.get_current_state(); }

        [[nodiscard]] auto solve() -> VoidError { return engine_.solve(); }

        [[nodiscard]] auto get_result() const -> const auto& { return engine_.get_result(); }

      private:
        Config config_;
        EngineType engine_;

        // std::vector<>
    };
} // namespace centipede
