#pragma once

#include "centipede/data/entry.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace centipede::writer
{
    /**
     * @brief Class for writing binary files.
     *
     * Data is written to the binary file via each entry, which contains different entrypoints (of the type
     * #centipede::EntryPoint). Before any operation, the #Binary::init() function must be called, where the file
     * handler is opened and internal buffer resetted, ready for the next data input. When adding each entrypoint, the
     * writer doesn't write the corresponding data to the binary file, but rather pushes the data to its internal buffer
     * (see @ref Binary::data_buffer_). Data is only written to the binary file after calling
     * #Binary::write_current_entry(). All entrypoints added before this call are grouped into the same entry.
     *
     * Configuration of the class is done via the Binary::Config struct.
     *
     * #### Example usage
     *
     * ```cpp
     * #include <centipede/centipede.hpp>
     * #include <print>
     *
     * auto writer = centipede::writer::Binary{};
     *
     * auto init_err = writer.init()
     *
     * if(not init_err.has_value())
     * {
     *      // Prints the error message here.
     *      std::println(stderr, "Error: {}", init_err.error());
     * }
     *
     * // Starts to write entrypoint
     * auto entry_point = get_new_entry_point();
     * auto add_err = writer.add_entrypoint(entry_point);
     *
     * // Deal with add_err here.
     * if(not add_err.has_value()) { ... }
     *
     * // Write current entry to the file.
     * auto write_err = writer.write_current_entry();
     *
     * // Deal with write_err here.
     * if(not write_err.has_value()) { ... }
     * ```
     * For a complete example, please check out test_writer.cpp.
     */
    class Binary
    {
      public:
        /**
         * @class Config
         * @brief Class for configuring the binary writer class (#Binary)
         *
         * #### Example usage
         *
         * ```cpp
         * auto writer =
         *     Binary{ Binary::Config{ .out_filename = "another_output.bin", .max_bufferpoint_size = 1000 } };
         * ```
         *
         */
        struct Config
        {
            static constexpr auto DEFAULT_BUFFER_SIZE = std::size_t{ 10000 };

            std::string out_filename = "output.bin";             //!< Output binary filename.
            uint32_t max_bufferpoint_size = DEFAULT_BUFFER_SIZE; //!< maximum bufferpoint for an entry.
        };

        using BufferType = std::pair<std::vector<uint32_t>, std::vector<float>>; //!< Type of the #data_buffer_.
        using BufferPoint = std::pair<uint32_t, float>; //!< Type of the buffer point stored in the #data_buffer_.

        /**
         * @brief Default constructor
         */
        Binary() = default;

        /**
         * @brief Constructor takes an argument for  the configuration.
         *
         * The config argument will be moved (`std::move`) to its member variable `config_`
         * @param config Configuration struct.
         * @see Config
         */
        constexpr explicit Binary(Config config)
            : config_{ std::move(config) }
        {
        }

        /**
         * @brief Initialization.
         *
         * The initialization function must be called before calling the #add_entrypoint() method. When calling this
         * function, #data_buffer_ is resetted and a file is opened with the specified name in #Config.
         * @return Returns ErrorCode::writer_file_fail_to_open if the file cannot be opened with the name specified by
         * Config::out_filename.
         * @see Config
         */
        [[nodiscard]] auto init() -> EnumError<>;

        /**
         * @brief Add an entrypoint to the internal data buffer.
         *
         * The adding of an entrypoint to the data buffer follows the sequence:
         * 1. If the sigma value is negative, returns immediately.
         * 2. Check if the current internal buffer has enough of space to store the latest data point.
         * 3. Add an index-value pair [0, measurement] as one buffer point to the data buffer.
         * 4. Add all local derivatives with indices as buffer points to the data buffer.
         * 5. Add an index-value pair [0, sigma] as one buffer point to the data buffer.
         * 6. Add all global derivatives with indices as buffer points to the data buffer.
         *
         * @param entry_point Input data structure which stores the local/global derivatives, measurement and error.
         * @return
         * - ErrorCode::writer_neg_or_zero_sigma if the sigma value is zero or negative.
         * - ErrorCode::writer_buffer_overflow if the buffer size is too small for the new entrypoint.
         * - ErrorCode::writer_entrypoint_rejected if the derivative values from the entrypoint are all zero.
         */
        template <std::size_t NLocals, std::size_t NGlobals>
        [[nodiscard]] auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>;

        /**
         * @brief Streaming an entry data to the output file.
         *
         * Streaming the entry data to the output file and call the reset() function to clear the internal buffer.
         * After the function is called, the writer is waiting for a new entry to be added.
         *
         * @return Number of bytes written to the binary file.
         */
        auto write_current_entry() -> EnumError<std::size_t>;

        /**
         * @brief Manually close the output file handler.
         *
         * This function will be called automatically when the destructor is called.
         */
        void close() { output_file_.close(); };

        /**
         * @brief Getter of the configuration.
         *
         * @return Returns a const reference to the member variable #config_.
         * @see ref
         */
        constexpr auto get_config() const -> const Config& { return config_; }

        /**
         * @brief Getter of the buffer.
         *
         * @return Returns a const reference to the member variable #data_buffer_.
         * @see ref
         */
        constexpr auto get_buffer() const -> const BufferType& { return data_buffer_; }

      private:
        bool has_entry_ = false;
        Config config_;             //!< Member variable for the configuration.
        BufferType data_buffer_;    //!< Data buffer to store entry_point
        std::ofstream output_file_; //!< Output file handler

        auto check_buffer_size(std::size_t size_to_add) const -> bool;
        auto write_to_binary() -> std::size_t;
        void reset();
        void resize_data_buffer(std::size_t size);
        auto fill_entrypoint_to_buffer(BufferPoint buffer_point, bool has_check_value = false) -> bool;
    };

    template <std::size_t NLocals, std::size_t NGlobals>
    auto Binary::add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> EnumError<>
    {
        assert(data_buffer_.first.size() == data_buffer_.second.size());

        if (entry_point.sigma <= 0.)
        {
            return std::unexpected{ ErrorCode::writer_neg_or_zero_sigma };
        }
        if (data_buffer_.first.empty())
        {
            return std::unexpected{ ErrorCode::writer_uninitialized };
        }
        if (not check_buffer_size(NLocals + NGlobals + 2))
        {
            return std::unexpected{ ErrorCode::writer_buffer_overflow };
        }

        auto old_size = data_buffer_.first.size();
        auto has_entry = false;

        fill_entrypoint_to_buffer(BufferPoint{ 0, entry_point.measurement });

        // NOTE: Can be changed to concat in C++26

        for (const auto& [idx, local_deriv] : std::views::zip(std::views::iota(0), entry_point.local_derivs))
        {
            has_entry |= fill_entrypoint_to_buffer(BufferPoint{ idx + 1, local_deriv }, true);
        }

        fill_entrypoint_to_buffer(BufferPoint{ 0, entry_point.sigma });

        for (const auto& [idx, global_deriv] : entry_point.global_derivs)
        {
            has_entry |= fill_entrypoint_to_buffer(BufferPoint{ idx + 1, global_deriv }, true);
        }

        has_entry_ |= has_entry;
        if (not has_entry)
        {
            resize_data_buffer(old_size);
            return std::unexpected{ ErrorCode::writer_entrypoint_rejected };
        }
        return {};
    }
} // namespace centipede::writer

namespace centipede
{
    using BinaryWriter = writer::Binary;
}
