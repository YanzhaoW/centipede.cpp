#pragma once

#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/master_engine.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>

namespace centipede::core
{

    /**
     * @brief Main frontend handler to the library
     *
     * This class should handle all inputs and configurations from users
     */
    template <typename DataType = double, engine::MasterOpt opt = {}>
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
        using EngineType = engine::Master<DataType, opt>;

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

        /**
         * @brief Initialization of the instance
         *
         */
        [[nodiscard]] auto init() -> EnumError<> { return {}; }

        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>
        {
            return engine_.add_entrypoint(entry_point);
        }

        auto analyze_current_entry() -> EnumError<std::size_t> { return {}; };

        [[nodiscard]] auto get_current_state() const -> const auto& { return engine_.get_current_state(); }

      private:
        Config config_;
        EngineType engine_;

        // std::vector<>
    };
} // namespace centipede::core
