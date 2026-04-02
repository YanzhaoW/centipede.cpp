#include "binary.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <fstream>
#include <ios>
#include <ranges>
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

        template <typename T>
            requires(sizeof(T) == sizeof(uint32_t))
        auto read_from_file(std::ifstream& input_file, std::vector<T>& data)
        {
            const auto read_size = data.size() * sizeof(T);
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            input_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(read_size));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
            return read_size;
        }

        enum class ReadingState : uint8_t
        {
            file_init,
            measurement,
            locals,
            sigma,
            globals,
            new_entrypoint,
            done
        };
    } // namespace

    auto Binary::init() -> EnumError<>
    {
        entry_buffer_.reserve(config_.max_bufferpoint_size);
        raw_entry_buffer_.first.reserve(config_.max_bufferpoint_size);
        raw_entry_buffer_.second.reserve(config_.max_bufferpoint_size);
        for ([[maybe_unused]] auto idx : std::views::iota(std::size_t{ 0 }, config_.max_bufferpoint_size))
        {
            entry_buffer_.emplace_back();
        }
        input_file_.open(config_.in_filename, std::ios::binary | std::ios::in);
        if (!input_file_.is_open())
        {
            return std::unexpected{ ErrorCode::reader_file_fail_to_open };
        }
        return {};
    }

    auto Binary::read_one_entry() -> EnumError<std::size_t>
    {
        if (entry_buffer_.empty() or !raw_entry_buffer_.first.empty() or !raw_entry_buffer_.second.empty() or
            size_ != 0U)
        {
            return std::unexpected{ ErrorCode::reader_uninitialized };
        }
        auto entry_size = uint32_t{};
        read_from_file(input_file_, entry_size);
        if (entry_size > config_.max_bufferpoint_size)
        {
            return std::unexpected{ ErrorCode::reader_buffer_overflow };
        }
        auto half_entry_size = uint32_t{ entry_size / 2U };
        raw_entry_buffer_.first.resize(half_entry_size);
        raw_entry_buffer_.second.resize(half_entry_size);
        read_from_file(input_file_, raw_entry_buffer_.second);
        read_from_file(input_file_, raw_entry_buffer_.first);

        auto entrypoint_counter = std::size_t{ 0 };

        auto current_state = ReadingState::file_init;

        for (auto [idx, data_index, data_value] :
             std::views::zip(std::views::iota(std::size_t{ 0 }, static_cast<std::size_t>(half_entry_size)),
                             raw_entry_buffer_.first,
                             raw_entry_buffer_.second))
        {
            assert(idx + 1U < half_entry_size);
            auto next_data_index = uint32_t{};
            if (idx + 1U < half_entry_size)
            {
                next_data_index = raw_entry_buffer_.first[idx + 1U];
            }
            switch (current_state)
            {
                case ReadingState::file_init:
                    if (data_index != 0U or next_data_index != 0U)
                    {
                        return std::unexpected{ ErrorCode::reader_file_fail_to_read };
                    }
                    current_state = ReadingState::measurement;
                    break;
                case ReadingState::done:
                    [[fallthrough]];
                case ReadingState::measurement:
                    assert(entrypoint_counter <= entry_buffer_.size());
                    entry_buffer_[entrypoint_counter].set_measurement(data_value);
                    current_state = ReadingState::locals;
                    break;
                case ReadingState::locals:
                    if (next_data_index == 0)
                    {
                        current_state = ReadingState::sigma;
                    }
                    entry_buffer_[entrypoint_counter].add_local(data_value);
                    break;
                case ReadingState::sigma:
                    assert(entrypoint_counter <= entry_buffer_.size());
                    entry_buffer_[entrypoint_counter].set_sigma(data_value);
                    current_state = ReadingState::globals;
                    break;
                case ReadingState::globals:
                    if (next_data_index == 0)
                    {
                        current_state = ReadingState::new_entrypoint;
                    }
                    assert(entrypoint_counter <= entry_buffer_.size());
                    entry_buffer_[entrypoint_counter].add_global(data_index - 1U, data_value);
                    break;
                case ReadingState::new_entrypoint:
                    current_state = ReadingState::measurement;
                    entrypoint_counter++;
                    break;
            }
        }
        current_state = ReadingState::done; // NOLINT(clang-analyzer-deadcode.DeadStores)
        size_ = entrypoint_counter;
        return entry_size + sizeof(entry_size);
    }

    void Binary::reset()
    {
        for (auto entrypoint : entry_buffer_)
        {
            entrypoint.reset();
        }
        raw_entry_buffer_.first.clear();
        raw_entry_buffer_.second.clear();
        size_ = 0U;
    }
} // namespace centipede::reader
