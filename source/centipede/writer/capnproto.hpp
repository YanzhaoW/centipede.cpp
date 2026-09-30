#pragma once

#include "centipede/data/entrypoint.hpp"
#include "centipede/util/return_types.hpp"
#include <capnp/message.h>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

namespace centipede::writer
{
    /**
     * @brief Class for writing capnproto data
     */
    class Capnproto
    {
      public:
        struct Config
        {
            std::string out_filename = "output.proto"; //!< Output binary filename.
        };

        /**
         * @brief Constructor with configuration
         * @param config Configuration struct
         */
        Capnproto(const Config& config)
            : config_{ config }
        {
        }

        [[nodiscard]] auto init() -> VoidError;

        template <std::size_t NLocals, std::size_t NGlobals>
        auto add_entrypoint(const EntryPoint<NLocals, NGlobals>& entry_point) -> VoidError;

        /**
         * @brief Getter of the configuration.
         *
         * @return Returns a const reference to the member variable #config_.
         */
        constexpr auto get_config() const -> const Config& { return config_; }

        constexpr auto get_buffer() const -> const std::vector<EntryPoint<>>& { return entrypoints_; }

        auto write_current_entry() -> ResultError<std::size_t>;

        void close() { output_file_.close(); };

      private:
        Config config_;
        std::ofstream output_file_;
        capnp::MallocMessageBuilder message_;
        std::vector<EntryPoint<>> entrypoints_;
    };

    template <std::size_t NLocals, std::size_t NGlobals>
    auto Capnproto::add_entrypoint(const EntryPoint<NLocals, NGlobals>& entrypoint) -> VoidError
    {
        entrypoints_.push_back(entrypoint);
        return {};
    }
} // namespace centipede::writer
