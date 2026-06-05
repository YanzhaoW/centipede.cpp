#pragma once

#include "centipede/core/engines/base_engine.hpp"
#include "centipede/core/engines/engine_types.hpp"
#include "centipede/core/engines/result.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <concepts>
#include <cstddef>

namespace centipede::core::engine
{

    /**
     * @brief Concept used for core::engine::Master option.
     */
    template <MatrixEngine engine_type, typename DataType>
    concept EngineLike = requires(Engine<engine_type, DataType> engine,
                                  Result<DataType>& result,
                                  typename Engine<engine_type, DataType>::Globals& globals) {
        typename Engine<engine_type, DataType>;
        typename Engine<engine_type, DataType>::Globals;
        { Engine<engine_type, DataType>{ std::size_t{ 0 } } };

        // { Engine<engine_type, DataType>::resize_globals(globals, std::size_t{}) } -> std::same_as<void>;
        { Engine<engine_type, DataType>::solve(globals, result) } -> std::same_as<void>;
        { engine.add_to_globals(globals) } -> std::same_as<void>;
        { engine.add_to_result(result) } -> std::same_as<void>;
        { engine.analyze(double{}) } -> std::same_as<VoidError>;
        { engine.fill_data(Entry<DataType>{}) } -> std::same_as<VoidError>;
    };

} // namespace centipede::core::engine
