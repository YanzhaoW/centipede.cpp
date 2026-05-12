#include "binary.hpp"
#include "centipede/data/entry.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <fstream>
#include <functional>
#include <ios>
#include <iterator>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

namespace centipede::reader
{
    namespace srs = std::ranges;
    namespace svs = std::views;
    namespace
    {
        template <typename T>
            requires(sizeof(T) == sizeof(uint32_t) and std::is_trivially_copyable_v<T>)
        auto read_from_file(std::ifstream& input_file, T& data) -> EnumError<std::size_t>
        {
            const auto read_size = sizeof(data);
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            input_file.read(reinterpret_cast<char*>(&data), static_cast<std::streamsize>(read_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            if (input_file.gcount() != static_cast<std::streamsize>(read_size))
            {
                return std::unexpected{ ErrorCode::reader_file_fail_to_read };
            }
            return read_size;
        }

        template <typename T>
            requires(sizeof(T) == sizeof(uint32_t) and std::is_trivially_copyable_v<T>)
        auto read_from_file(std::ifstream& input_file, std::vector<T>& data) -> EnumError<std::size_t>
        {
            assert(!data.empty());
            const auto read_size = data.size() * sizeof(T);
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            input_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(read_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            if (input_file.gcount() != static_cast<std::streamsize>(read_size))
            {
                return std::unexpected{ ErrorCode::reader_file_fail_to_read };
            }
            return read_size;
        }

        template <typename IterCursor, typename IterEnd>
        struct ChunkPointer
        {
            IterCursor iter;
            IterEnd end;
            EntryPoint<>* entrypoint;
            std::size_t& current_size;
        };

        auto chunk_check_size_one(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            return (srs::size(*(chunk_ptr.iter)) == 1U) ? std::optional{ chunk_ptr } : std::nullopt;
        }

        auto chunk_not_end_and_increment(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            return (++(chunk_ptr.iter) != chunk_ptr.end) ? std::optional{ chunk_ptr } : std::nullopt;
        }

        auto chunk_handle_measurement(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            (chunk_ptr.entrypoint)->set_measurement(std::get<1>(*(*(chunk_ptr.iter)).begin()));
            return chunk_ptr;
        }

        auto chunk_handle_globals(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            for (const auto& global : *(chunk_ptr.iter))
            {
                chunk_ptr.entrypoint->add_global(std::get<0>(global), std::get<1>(global));
            }
            return chunk_ptr;
        }

        auto chunk_handle_sigma(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            chunk_ptr.entrypoint->set_sigma(std::get<1>(*(*(chunk_ptr.iter)).begin()));
            return chunk_ptr;
        }

        auto chunk_handle_locals(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            for (const auto& local : *(chunk_ptr.iter) | svs::values)
            {
                chunk_ptr.entrypoint->add_local(local);
            }
            ++(chunk_ptr.current_size);
            return chunk_ptr;
        }

        auto chunk_end_after_increment(auto chunk_ptr)
        {
            assert(chunk_ptr.entrypoint != nullptr);
            return (++(chunk_ptr.iter) == chunk_ptr.end) ? std::optional{ chunk_ptr } : std::nullopt;
        }

        auto parse_entry_points(const Binary::RawBufferType& input, Binary::BufferType& output)
            -> EnumError<std::size_t>
        {
            if (input.first.at(0) != 0U)
            {
                return std::unexpected{ ErrorCode::reader_file_fail_to_read };
            }
            constexpr auto chunk_size{ 4 };
            auto size = std::size_t{};
            auto zipped = svs::zip(input.first, input.second) | svs::drop(1) |
                          svs::chunk_by([](const auto& current, const auto& next) -> auto
                                        { return std::get<0>(current) != 0U and std::get<0>(next) != 0U; });
            if (zipped.begin() == zipped.end())
            {
                return std::unexpected{ ErrorCode::reader_file_fail_to_read };
            }
            // TODO: Use chunk_view after libc++ supports it.
            auto chunks =
                svs::zip(svs::iota(0), zipped) |
                svs::chunk_by([](const auto& current, const auto& next) -> auto
                              { return std::get<0>(current) / chunk_size == std::get<0>(next) / chunk_size; }) |
                svs::transform([](auto&& chunk) -> auto { return chunk | svs::values; });

            auto is_ok = srs::all_of(svs::zip_transform(
                                         [&size](auto&& chunk, auto&& entrypoint) -> bool
                                         {
                                             auto chunk_ptr = ChunkPointer{ .iter = chunk.begin(),
                                                                            .end = chunk.end(),
                                                                            .entrypoint = &entrypoint,
                                                                            .current_size = size };
                                             using ChunkPtrType = decltype(chunk_ptr);
                                             return chunk_check_size_one(chunk_ptr)
                                                 .transform(chunk_handle_measurement<ChunkPtrType>)
                                                 .and_then(chunk_not_end_and_increment<ChunkPtrType>)
                                                 .transform(chunk_handle_globals<ChunkPtrType>)
                                                 .and_then(chunk_not_end_and_increment<ChunkPtrType>)
                                                 .and_then(chunk_check_size_one<ChunkPtrType>)
                                                 .transform(chunk_handle_sigma<ChunkPtrType>)
                                                 .and_then(chunk_not_end_and_increment<ChunkPtrType>)
                                                 .transform(chunk_handle_locals<ChunkPtrType>)
                                                 .and_then(chunk_end_after_increment<ChunkPtrType>)
                                                 .has_value();
                                         },
                                         chunks,
                                         output),
                                     std::identity{});
            if (not is_ok)
            {
                return std::unexpected{ ErrorCode::reader_file_fail_to_read };
            }
            return size;
        }
    } // namespace

    auto Binary::init() -> EnumError<>
    {
        if (config_.in_filename.empty())
        {
            return std::unexpected{ ErrorCode::reader_invalid_filename };
        }
        entry_buffer_.resize(config_.max_bufferpoint_size);
        raw_entry_buffer_.first.reserve(config_.max_bufferpoint_size);
        raw_entry_buffer_.second.reserve(config_.max_bufferpoint_size);
        input_file_.open(config_.in_filename, std::ios::binary | std::ios::in);
        if (!input_file_.is_open())
        {
            return std::unexpected{ ErrorCode::reader_file_fail_to_open };
        }
        n_entries_ = 0Z;
        end_of_file_ = false;
        return {};
    }

    auto Binary::read_one_entry() -> EnumError<std::size_t>
    {
        if (entry_buffer_.empty())
        {
            return std::unexpected{ ErrorCode::reader_uninitialized };
        }
        reset();
        auto read_size = uint32_t{};
        if (auto result = read_from_file(input_file_, read_size); !result)
        {
            if (input_file_.eof() and input_file_.gcount() == 0)
            {
                end_of_file_ = true;
                return 0U;
            }
            return std::unexpected{ result.error() };
        }
        if (auto result = read_entry_to_buffer(read_size); !result)
        {
            return std::unexpected{ result.error() };
        }
        if (read_size > config_.max_bufferpoint_size)
        {
            entry_buffer_.resize(read_size);
        }
        auto size = parse_entry_points(raw_entry_buffer_, entry_buffer_);
        if (not size)
        {
            return std::unexpected{ size.error() };
        }
        ++n_entries_;
        size_ = size.value();
        return size.value();
    }

    void Binary::reset()
    {
        for (auto& entrypoint : entry_buffer_)
        {
            entrypoint.reset();
        }
        raw_entry_buffer_.first.clear();
        raw_entry_buffer_.second.clear();
        size_ = 0U;
    }

    auto Binary::read_entry_to_buffer(uint32_t read_size) -> EnumError<>
    {
        raw_entry_buffer_.first.resize(read_size / 2U);
        raw_entry_buffer_.second.resize(read_size / 2U);
        if (auto result = read_from_file(input_file_, raw_entry_buffer_.second); !result)
        {
            return std::unexpected{ result.error() };
        }
        if (auto result = read_from_file(input_file_, raw_entry_buffer_.first); !result)
        {
            return std::unexpected{ result.error() };
        }
        return {};
    }
} // namespace centipede::reader
