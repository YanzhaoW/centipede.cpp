#pragma once

#include "centipede/data/entrypoint.hpp"
#include "centipede/util/common_definitions.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace centipede::reader
{
    /**
     * @class Binary
     * @brief Class for reading binary files entry-wise.
     *
     * Data is read from the binary file entry-wise. Each entry contains multiple
     * entrypoints of type #centipede::EntryPoint. Before reading,
     * #centipede::reader::Binary::init() must be called to open the file and
     * initialize the internal buffers.
     *
     * The reader can be used as an input range. Each iteration reads one entry
     * from the file and returns an `EntrySpan` referencing the current entry stored
     * in the internal buffers.
     *
     * Iteration stops automatically once end-of-file is reached or a read/parsing
     * error occurs. The final reader state can be queried afterwards via
     * #centipede::reader::Binary::get_status().
     *
     * Note that manual reading via
     * #centipede::reader::Binary::read_one_entry() and
     * #centipede::reader::Binary::get_current_entry() is also supported.
     *
     * Configuration of the class is done via Binary::Config.
     *
     * #### Example usage
     *
     * ```cpp
     * auto reader = centipede::reader::Binary{
     *     centipede::reader::Binary::Config{ .in_filename = "output.bin" }
     * };
     *
     * auto init_err = reader.init();
     * if (not init_err.has_value())
     * {
     *     std::println(stderr, "Error: {}", init_err.error());
     *     return EXIT_FAILURE;
     * }
     *
     * for (const auto& entry : reader)
     * {
     *     for (const auto& entrypoint : entry)
     *     {
     *         // handle entrypoint
     *     }
     * }
     *
     * if (not reader.is_ok())
     * {
     *     std::println(stderr, "Error: {}", reader.get_status());
     * }
     *
     * std::println("N Entries: {}", reader.get_n_entries());
     * ```
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
         * auto reader =
         *     Binary{ Binary::Config{ .in_filename = "output.bin", .max_bufferpoint_size = 1000 } };
         * ```
         *
         */
        struct Config
        {
            std::string in_filename;                                     //!< Input binary filename.
            uint32_t max_bufferpoint_size = common::DEFAULT_BUFFER_SIZE; //!< maximum bufferpoint for an entry.
        };
        using RawBufferType = std::pair<std::vector<uint32_t>, std::vector<float>>; //!< Type of #raw_entry_buffer_
        using BufferType = std::vector<EntryPoint<>>;                               //!< Type of #entry_buffer_

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
         * The initialization function must be called before calling the #read_one_entry() method. When calling this
         * function, #raw_entry_buffer_ and #entry_buffer_ get resized and a file is opened with the specified name in
         * #Config.
         * @return Returns ErrorCode::reader_file_fail_to_open if the file cannot be opened with the name specified by
         * Config::in_filename.
         * @see Config
         */
        [[nodiscard]] auto init() -> EnumError<>;

        /**
         * @brief Manually close the input file handler.
         *
         * This function will be called automatically when the destructor is called.
         */
        void close() { input_file_.close(); }

        /**
         * @brief Reads one entry from file into the internal buffers.
         *
         * Reading an entry follows this crude sequence:
         * 1. if #init() is not called once after instanciating, returns an error
         * 2. Reads one entry to #raw_entry_buffer_, returns if read operation fails.
         * 3. Parses entry and stores individual entrypoints in #entry_buffer_, returns if file format is corrupted.
         * 4. Increases #n_entries_ for one entry is read and sets #size_ corresponding to the number of 32 Bit values
         *     (global and local derivs, sigmas and measurements) contained in current entry.
         * 5. returns #size_
         *
         * @return
         * - ErrorCode::reader_uninitialized if #init() is not called before reading
         * - ErrorCode::reader_buffer_overflow if buffer size is too small.
         * - ErrorCode::reader_file_fail_to_read if the file stream is broken or file format is corrupted.
         * - #size_ on success
         */
        [[maybe_unused]] auto read_one_entry() -> EnumError<std::size_t>;

        /**
         * @brief Getter of #entry_buffer_.
         *
         * @return Returns a std::span of #entry_buffer_
         **/
        [[nodiscard]] auto get_current_entry() const -> auto { return std::span{ entry_buffer_.begin(), size_ }; }

        /**
         * @brief Getter of the configuration.
         *
         * @return Returns a const reference to the member variable #config_.
         * @see ref
         */
        [[nodiscard]] constexpr auto get_config() const -> const Config& { return config_; }

        /**
         * @brief Getter of n_entries_
         *
         * @return Total number of entries read by the current instance
         */
        [[nodiscard]] constexpr auto get_n_entries() const -> std::size_t { return n_entries_; }

        /**
         * @brief Checks if last read operation reached end of file.
         * @return Returns true if end of file is reached.
         */
        [[nodiscard]] auto is_end_of_file() const -> bool { return end_of_file_; }

        /**
         * @brief Returns the current reader status.
         *
         * The status is updated during iteration and after manual read operations.
         *
         * @return
         * - ErrorCode::invalid while iteration/reading is in progress.
         * - ErrorCode::success if iteration finished successfully.
         * - Any other ErrorCode if a read or parsing error occurred.
         */
        [[nodiscard]] auto get_status() const -> ErrorCode { return status_; }

        /**
         * @brief Returns true if the last read was successful.
         *
         * Note that this while return false on read error as well as incompleted read operation.
         */
        [[nodiscard]] auto is_ok() const -> bool { return get_status() == ErrorCode::success; }

        using EntrySpan = std::span<const BufferType::value_type>;

        /**
         * @brief Sentinel type marking the end of a Binary reader range.
         *
         * The sentinel does not store state itself. End detection is handled by
         * Binary::Iterator, which stops once EOF or an empty read is reached.
         */
        class Sentinel
        {
        };

        /**
         * @brief Input iterator for entry-wise reading of a binary file.
         *
         * The iterator reads entries from the associated Binary reader. On
         * construction, the first entry is read. Each increment reads the next entry.
         *
         * Dereferencing returns an EntrySpan referencing the current entry stored in
         * the reader's internal buffer.
         *
         * Iteration stops automatically once end-of-file is reached or a read error
         * occurs. The final reader state can be queried afterwards via
         * #Binary::get_status().
         *
         * The returned span is valid only until the iterator is incremented, because
         * incrementing reads the next entry and resets/reuses the internal buffers.
         */
        class Iterator
        {
          public:
            /**
             * @brief Constructs an iterator for the given Binary reader.
             *
             * Construction performs the first read operation.
             *
             * @param reader_ptr Pointer to the associated Binary reader instance.
             */
            explicit Iterator(Binary* reader_ptr)
                : reader_{ reader_ptr }
            {
                reader_->status_ = ErrorCode::invalid;
                ++(*this);
            }

            using iterator_category = std::input_iterator_tag; //!< Iterator category type.
            using difference_type = std::ptrdiff_t;            //!< Difference type.
            using value_type = EntrySpan;                      //!< Dereferenced value type.
            using reference = const EntrySpan&;                //!< Dereference reference type.

            /**
             * @brief Dereferences the iterator.
             *
             * @return Returns the current EntrySpan.
             */
            auto operator*() const -> const EntrySpan& { return current_; }

            /**
             * @brief Advances the iterator to the next entry.
             *
             * Reads the next entry from the underlying Binary reader and updates the
             * internal state accordingly.
             *
             * @return Reference to the incremented iterator.
             */
            auto operator++() -> Iterator&
            {
                auto result = reader_->read_one_entry();

                if (not result.has_value())
                {
                    reader_->status_ = result.error();
                    return *this;
                }

                if (reader_->is_end_of_file() or result.value() == 0U)
                {
                    reader_->status_ = ErrorCode::success;
                    return *this;
                }

                current_ = reader_->get_current_entry();
                reader_->status_ = ErrorCode::invalid;
                return *this;
            }
            /**
             * @brief Post-increment operator.
             *
             * @return Copy of the iterator before incrementing.
             */
            auto operator++(int) -> Iterator
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            /**
             * @brief Compares iterator against the end sentinel.
             *
             * @return Returns true while iteration is not finished.
             */
            auto operator!=(const Sentinel&) const -> bool { return reader_->status_ == ErrorCode::invalid; }

          private:
            Binary* reader_{};    //!< Associated Binary reader instance.
            EntrySpan current_{}; //!< Current iterator value.
        };

        /**
         * @brief Returns an iterator positioned at the first readable entry.
         *
         * @return Iterator initialized with the first entry read from file.
         */
        auto begin() -> Iterator { return Iterator{ this }; }

        /**
         * @brief Returns the end sentinel of the Binary reader range.
         *
         * @return Sentinel representing the end of the range.
         */
        auto end() const -> Sentinel { return Sentinel{}; }

      private:
        BufferType entry_buffer_;        //!< A vector containing all entrypoints of the current entry.
        RawBufferType raw_entry_buffer_; //!< A buffer to store raw data coming from file stream.
        Config config_;                  //!< Member variable for the configuration.
        std::ifstream input_file_;       //!< Input file handler
        std::size_t size_{};             //!< Number of Entrypoints in the current entry
        std::size_t n_entries_{};        //!< Total number of entries read by this instance
        bool end_of_file_{ false };      //!< Indicates if end of file is reached. Gets updated on read.
        ErrorCode status_{ ErrorCode::invalid };

        void reset();
        auto read_entry_to_buffer(uint32_t read_size) -> EnumError<>;
    };
} // namespace centipede::reader
