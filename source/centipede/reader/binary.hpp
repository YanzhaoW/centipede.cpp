#pragma once

#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

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
            static constexpr auto DEFAULT_BUFFER_SIZE = std::size_t{ 10000 };
            std::string in_filename = "output.bin";              //!< Input binary filename.
            uint32_t max_bufferpoint_size = DEFAULT_BUFFER_SIZE; //!< maximum bufferpoint for an entry.
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

        [[maybe_unused]] auto read_one_entry() -> EnumError<std::size_t>;

        [[nodiscard]] auto get_current_entry() const -> const auto& { return entry_buffer_; }

      private:
        std::vector<EntryPoint<>> entry_buffer_; //!< A vector containing all entrypoints of the current entry.
        Config config_;
        std::ofstream input_file_; //!< Input file handler
        uint32_t total_size_{};    //!< Total amount of entrypoints in the file
    };
} // namespace centipede::reader
