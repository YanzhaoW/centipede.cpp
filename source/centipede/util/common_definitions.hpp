#pragma once

#include <cstddef>

namespace centipede::common
{
    constexpr auto DEFAULT_BUFFER_SIZE =
        std::size_t{ 10000 }; //!< Default maximum buffer size for binary readers/writers.
    constexpr auto DEFAULT_INDICATOR_BAR_WIDTH = std::size_t{ 50 };
} // namespace centipede::common
