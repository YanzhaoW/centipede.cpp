#pragma once

#include "centipede/data/entry.hpp"
#include "centipede/util/common_definitions.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <span>
#include <string>
#include <utility>
#include <vector>

// TODO: Add documentation
// TODO: Add integration and unit tests

namespace centipede::internal
{

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
}

namespace centipede::reader
{
    /**
     * @class Binary
     * @brief Class for reading binary files.
     * To be added ...
     *
     * To be added ...
     */
    class Binary
    {
      public:
        /**
         * @class Config
         * @brief Configuration for the class.
         * To be added ...
         *
         * To be added ...
         */
        struct Config
        {
            std::string in_filename = "output.bin";                      //!< Input binary filename.
            uint32_t max_bufferpoint_size = common::DEFAULT_BUFFER_SIZE; //!< maximum bufferpoint for an entry.
        };

        /**
         * @brief Default constructor.
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
         * @brief Initialization of the instance.
         *
         * detailed description
         * @return name description
         */
        [[nodiscard]] auto init() -> EnumError<>;

        void cloise()
        {
            current_state_ = internal::ReadingState::file_init;
            input_file_.close();
        }

        [[maybe_unused]] auto read_one_entry() -> EnumError<std::size_t>;

        [[nodiscard]] auto get_current_entry() const -> auto { return std::span{ entry_buffer_ }.subspan(size_); }

      private:
        std::vector<EntryPoint<>> entry_buffer_; //!< A vector containing all entrypoints of the current entry.
        std::pair<std::vector<uint32_t>, std::vector<float>> raw_entry_buffer_;
        std::pair<std::vector<uint32_t>, std::vector<float>> entrypoint_buffer_;
        Config config_;            //!< Member variable for the configuration.
        std::ifstream input_file_; //!< Input file handler
        std::size_t size_{};       //!< Number of Entrypoints in the current entry
        internal::ReadingState current_state_ = internal::ReadingState::file_init;

        void reset();
        void read_entry_to_buffer(uint32_t read_size);
    };
} // namespace centipede::reader
