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
#include <type_traits>
#include <utility>
#include <vector>

namespace centipede::reader
{
    namespace
    {
        template <typename T>
            requires(sizeof(T) == sizeof(uint32_t))
        auto read_from_file(std::ifstream& input_file, T& data)
        {
            const auto read_size = sizeof(uint32_t);
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            input_file.read(reinterpret_cast<char*>(&data), static_cast<std::streamsize>(read_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            return read_size;
        }

        auto read_size_from_file(std::ifstream& input_file) -> uint32_t
        {
            auto size = uint32_t{};
            read_from_file(input_file, size);
            return size;
        }

        // auto read_raw_entry(std::ifstream& input_file, ) {}

    } // namespace

    auto Binary::init() -> EnumError<>
    {
        reset();
        entry_buffer_.reserve(config_.max_bufferpoint_size);
        input_file_.open(config_.in_filename, std::ios::binary | std::ios::in);
        if (!input_file_.is_open())
        {
            return std::unexpected{ ErrorCode::reader_file_fail_to_open };
        }
        return {};
    }

    auto Binary::read_one_entry() -> EnumError<std::size_t> { auto entry_size = read_size_from_file(input_file_); }

    void Binary::reset() { entry_buffer_.clear(); }
} // namespace centipede::reader
