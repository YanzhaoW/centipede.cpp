#pragma once

#include "centipede/data/entry.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <string>
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
         *  @brief Configuration for the class.
         *  To be added ...
         *
         * To be added ...
         */
        struct Config
        {
            std::string in_filename = "output.bin"; //!< Output binary filename.
        };

        /**
         * @brief Default constructor.
         */
        Binary() = default;

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
    };
} // namespace centipede::reader
