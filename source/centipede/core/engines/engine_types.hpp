#pragma once

#include <cstdint>

namespace centipede::core::engine
{
    /**
     * @brief Engine types.
     *
     */
    enum class MatrixEngineType : uint8_t
    {
        eigen,
        xtensor,
        mock,
    };

    /**
     * @brief Compile-time options for the master engine class
     */
    struct MasterOpt
    {
        MatrixEngineType engine_type = MatrixEngineType::eigen;
        bool has_multi_slaves = false; //!< Use taskflow or not.
    };

} // namespace centipede::core::engine

namespace centipede
{
    using EngineType = core::engine::MatrixEngineType;
}
