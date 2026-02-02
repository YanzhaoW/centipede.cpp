#include "binary.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <fstream>
#include <ios>
#include <vector>

namespace centipede::writer
{

    namespace
    {
        template <typename T>
        inline auto write_to_file(std::ofstream& output_file, const T& data)
        {
            const auto write_size = sizeof(T);
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            output_file.write(reinterpret_cast<const char*>(&data), static_cast<std::streamsize>(write_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            return write_size;
        }

        template <typename T>
        inline auto write_to_file(std::ofstream& output_file, const std::vector<T>& data)
        {
            const auto write_size = sizeof(T) * data.size();
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            output_file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(write_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            return write_size;
        }
    } // namespace

    auto Binary::fill_entrypoint_to_buffer(BufferPoint buffer_point, bool has_check_value) -> bool
    {
        if ((not has_check_value) or buffer_point.second != 0)
        {
            data_buffer_.first.push_back(buffer_point.first);
            data_buffer_.second.push_back(buffer_point.second);
            return true;
        }
        return false;
    }

    auto Binary::init() -> EnumError<>
    {
        data_buffer_.first.reserve(config_.max_bufferpoint_size);
        data_buffer_.second.reserve(config_.max_bufferpoint_size);
        reset();
        output_file_.open(config_.out_filename, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!output_file_.is_open())
        {
            return std::unexpected{ ErrorCode::writer_file_fail_to_open };
        }
        return {};
    }

    auto Binary::write_current_entry() -> EnumError<std::size_t>
    {
        assert(data_buffer_.first.size() == data_buffer_.second.size());
        if (data_buffer_.first.empty())
        {
            return std::unexpected{ ErrorCode::writer_uninitialized };
        }
        if (not has_entry_)
        {
            return 0;
        }
        auto written_size = write_to_binary();
        reset();
        return written_size;
    }

    auto Binary::write_to_binary() -> std::size_t
    {
        assert(data_buffer_.first.size() == data_buffer_.second.size());
        const auto data_size = static_cast<uint32_t>((data_buffer_.first.size()) + (data_buffer_.second.size()));
        auto total_written_size = std::size_t{ 0 };
        total_written_size += write_to_file(output_file_, data_size);
        total_written_size += write_to_file(output_file_, data_buffer_.second);
        total_written_size += write_to_file(output_file_, data_buffer_.first);
        return total_written_size;
    }

    void Binary::resize_data_buffer(std::size_t size)
    {
        assert(size <= data_buffer_.first.size());
        assert(data_buffer_.first.size() == data_buffer_.second.size());

        data_buffer_.first.resize(size);
        data_buffer_.second.resize(size);
    }

    void Binary::reset()
    {
        data_buffer_.first.resize(1, 0);
        data_buffer_.second.resize(1, 0.F);
        has_entry_ = false;
    }

    auto Binary::check_buffer_size(std::size_t size_to_add) const -> bool
    {
        return data_buffer_.first.size() + size_to_add < config_.max_bufferpoint_size;
    }
} // namespace centipede::writer
