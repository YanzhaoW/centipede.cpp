#pragma once

#include "centipede/centipede.hpp"
#include "centipede/cli/config.hpp"
#include "centipede/cli/lua_connector.hpp"
#include "centipede/util/return_types.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace centipede
{

    /**
     * @brief Application class for command line interface.
     *
     */
    class Application
    {
      public:
        using Config = cli::Config;

        ~Application() = default;
        Application(const Application&) = delete;
        Application(Application&&) = default;
        auto operator=(const Application&) -> Application& = delete;
        auto operator=(Application&&) -> Application& = default;

        /**
         * @brief Constructor with configurations
         *
         * @param config Configuration struct
         */
        explicit Application(Config config)
            : config_{ std::move(config) }
        {
        }

        /**
         * @brief Initialize the application.
         *
         * @return Status of the result
         */
        auto init() -> VoidStr;

        /**
         * @brief Set the configurations from the lua file
         *
         * @param filename Lua file name
         * @return Status of the result
         */
        auto use_config(const std::string& filename) -> VoidStr;

        /**
         * @brief Start reading the data and analysis. This method should only be called after init().
         *
         * @return Status of the result
         */
        auto run() -> VoidStr;

        auto save_output() -> VoidStr;

      private:
        using Reader = std::variant<reader::Binary>;
        using Handlers = std::variant<Handler<float, { .engine_type = MatrixEngine::eigen }>>;
        cli::LuaConnector lua_connector_{};
        Config config_;
        Reader reader_;
        std::vector<Result<float>> results_;
        std::unique_ptr<Handlers> handle_;

        auto run_once(std::size_t run_idx, auto& reader, auto& handle) -> VoidStr;

        auto set_global_initial_values_from_file(const cli::Config::Input::InitPar& init_config) -> VoidStr;

        void set_global_initial_values(auto& handle);
    };
} // namespace centipede
