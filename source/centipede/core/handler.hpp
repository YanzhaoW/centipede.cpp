#pragma once

#include "centipede/core/engines/base_engine.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>

namespace centipede::core
{

    /**
     * @brief Compile time configuration for Handler classes
     */
    struct HandlerConfig
    {
        engines::MatrixEngineType engine_type = engines::MatrixEngineType::eigen; //!< engine types.
    };

    /**
     * @brief Main frontend handler to the library
     *
     * This class should handle all inputs and configurations from users
     */
    template <typename DataType = double, HandlerConfig Config = HandlerConfig{}>
    class Handler
    {
      public:
        /**
         * @brief Constructor
         *
         * This is the default constructor
         */
        Handler() = default;

        /**
         * @brief Initialization of the instance
         *
         */
        [[nodiscard]] auto init() -> EnumError<>;

        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>;

        auto analyze_current_entry() -> EnumError<std::size_t>;

      private:
        engines::Engine<Config.engine_type, DataType> engine_;

        // std::vector<>
    };
} // namespace centipede::core
