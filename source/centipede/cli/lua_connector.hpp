#pragma once

#include "centipede/util/return_types.hpp"
#include <sol/state.hpp>
#include <string>

namespace centipede::cli
{
    struct Config;
    /**
     * @brief Class for communicating the lua state machine.
     */
    class LuaConnector
    {
      public:
        LuaConnector() = default;

        auto init(Config& config) -> VoidStr;

        auto read_user_config(const std::string& filename, Config& config) -> VoidStr;

      private:
        sol::state lua_state_;
        std::string warn_msg_stream_;

        auto setup_lua_pkg_path() -> VoidStr;
        void setup_lua_functions(Config& config);
        void setup_lua_warn_msg();
        void setup_lua_usr_types();
        void read_struct_from_user_config(Config& config);
    };
} // namespace centipede::cli
