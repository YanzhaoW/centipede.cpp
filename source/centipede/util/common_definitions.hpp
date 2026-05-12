#pragma once

#include <cstddef>

namespace centipede::common
{
    constexpr auto DEFAULT_BUFFER_SIZE =
        std::size_t{ 10000 }; //!< Default maximum buffer size for binary readers/writers.
} // namespace centipede::common
