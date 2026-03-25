#include "binary.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <ios>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace centipede::reader
{
    namespace
    {

    }

    auto Binary::init() -> EnumError<>
    {
        // reset();
        input_file_.open(config_.in_filename, std::ios::binary | std::ios::in);
        if (!input_file_.is_open())
        {
            return std::unexpected{ ErrorCode::reader_file_fail_to_open };
        }
        return {};
    }

    auto Binary::read_one_entry() -> EnumError<std::size_t> {}

} // namespace centipede::reader
