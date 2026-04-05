#include "binary.hpp"
#include "centipede/data/entry.hpp"
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

        struct ReadingFrame
        {
            uint32_t index{};
            uint32_t next_index{};
            float value{};
        };

        void handle_measurement(const ReadingFrame& current_frame,
                                EntryPoint<>& current_entrypoint,
                                internal::ReadingState& current_state)
        {
            current_entrypoint.set_measurement(current_frame.value);
            current_state = internal::ReadingState::locals;
        }

        void handle_locals(const ReadingFrame& current_frame,
                           EntryPoint<>& current_entrypoint,
                           internal::ReadingState& current_state)
        {
            if (current_frame.next_index == 0)
            {
                current_state = internal::ReadingState::sigma;
            }
            current_entrypoint.add_local(current_frame.value);
        }

        void handle_sigma(const ReadingFrame& current_frame,
                          EntryPoint<>& current_entrypoint,
                          internal::ReadingState& current_state)
        {
            current_entrypoint.set_sigma(current_frame.value);
            current_state = internal::ReadingState::globals;
        }

        void handle_globals(const ReadingFrame& current_frame,
                            EntryPoint<>& current_entrypoint,
                            internal::ReadingState& current_state)
        {
            if (current_frame.next_index == 0)
            {
                current_state = internal::ReadingState::new_entrypoint;
            }
            current_entrypoint.add_global(current_frame.index - 1U, current_frame.value);
        }

        auto handle_state(const ReadingFrame& current_frame,
                          EntryPoint<>& current_entrypoint,
                          internal::ReadingState& current_state) -> bool
        {
            switch (current_state)
            {
                case internal::ReadingState::file_init:
                    current_state = internal::ReadingState::measurement;
                    break;
                case internal::ReadingState::done:
                    [[fallthrough]];
                case internal::ReadingState::measurement:
                    handle_measurement(current_frame, current_entrypoint, current_state);
                    break;
                case internal::ReadingState::locals:
                    handle_locals(current_frame, current_entrypoint, current_state);
                    break;
                case internal::ReadingState::sigma:
                    handle_sigma(current_frame, current_entrypoint, current_state);
                    break;
                case internal::ReadingState::globals:
                    handle_globals(current_frame, current_entrypoint, current_state);
                    break;
                case internal::ReadingState::new_entrypoint:
                    current_state = internal::ReadingState::measurement;
                    return true;
            }
            return false;
        }
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
        if (!input_file_.is_open() or current_state_ != internal::ReadingState::file_init)
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
        read_entry_to_buffer(entry_size);
        auto half_entry_size = entry_size / 2U;
        auto entrypoint_counter = std::size_t{ 0 };
        auto current_frame = ReadingFrame{};
        for (auto [idx, data_index, data_value] :
             std::views::zip(std::views::iota(std::size_t{ 0 }, static_cast<std::size_t>(half_entry_size)),
                             raw_entry_buffer_.first,
                             raw_entry_buffer_.second))
        {
            auto next_data_index = uint32_t{};
            if (idx + 1U < half_entry_size)
            {
                next_data_index = raw_entry_buffer_.first[idx + 1U];
            }
            current_frame = ReadingFrame{ .index = data_index, .next_index = next_data_index, .value = data_value };
            if (handle_state(current_frame, entry_buffer_[entrypoint_counter], current_state_))
            {
                entrypoint_counter++;
            }
        }
        // NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
        current_state_ = internal::ReadingState::done;
        // NOLINTEND(clang-analyzer-deadcode.DeadStores)
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

    void Binary::read_entry_to_buffer(uint32_t read_size)
    {
        raw_entry_buffer_.first.resize(read_size / 2U);
        raw_entry_buffer_.second.resize(read_size / 2U);
        read_from_file(input_file_, raw_entry_buffer_.second);
        read_from_file(input_file_, raw_entry_buffer_.first);
    }
} // namespace centipede::reader
