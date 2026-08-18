#include "lua_connector.hpp"
#include "centipede/cli/config.hpp"
#include "centipede/cli/location_identifier.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/return_types.hpp"
#include <expected>
#include <filesystem>
#include <format>
#include <lua.h>
#include <sol/error.hpp>
#include <sol/property.hpp>
#include <sol/table.hpp>
#include <sol/types.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <system_error>
#include <vector>

namespace centipede::cli
{

    auto LuaConnector::init(Config& config) -> VoidStr
    {
        lua_state_.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math, sol::lib::table);
        return setup_lua_pkg_path().transform(
            [this, &config] -> void
            {
                setup_lua_functions(config);
                setup_lua_warn_msg();
                setup_lua_usr_types();
            });
    }

    auto LuaConnector::read_user_config(const std::string& filename, Config& config) -> VoidStr
    {
        auto res = lua_state_.safe_script_file(filename);
        if (not res.valid())
        {
            return std::unexpected{ std::format(
                "Error occurred when reading the lua file {:?}: \n\t{}", filename, res.get<sol::error>().what()) };
        }
        read_struct_from_user_config(config);
        return {};
    }

    auto LuaConnector::setup_lua_pkg_path() -> VoidStr
    {

        return get_current_exe_location().and_then(
            [this](const auto& current_exe_path) -> VoidStr
            {
                spdlog::debug("Successfully identified the current executable path: {}", current_exe_path.c_str());
                const auto rel_dir_paths = std::vector<std::filesystem::path>{ "../../scripts" };

                for (const auto& rel_path : rel_dir_paths)
                {

                    auto err = std::error_code{};
                    auto folder = std::filesystem::weakly_canonical(current_exe_path.parent_path() / rel_path, err);
                    spdlog::debug("Lua: Adding {} to global package path.", folder.c_str());
                    lua_state_["package"]["path"] =
                        std::format("{}/?.lua;", folder.c_str()) + lua_state_["package"]["path"].get<std::string>();

                    if (err)
                    {
                        return std::unexpected{ err.message() };
                    }
                }
                return {};
            });
    }

    void LuaConnector::read_struct_from_user_config(Config& config)
    {
        auto lua_pkg = lua_state_["package"]["loaded"]["centipede"];
        auto lua_config = lua_pkg["config"].get<sol::table>();

        read_lua_table(lua_config, config);
    }

    void LuaConnector::setup_lua_functions(Config& config)
    {
        lua_state_.set_function("get_default_config",
                                [&config, this]() -> sol::table { return create_lua_table(lua_state_, config); });
    }

    void LuaConnector::setup_lua_usr_types()
    {
        lua_state_.new_usertype<EntryPoint<>>(
            "EntryPoint",
            "locals",
            sol::readonly_property([](const EntryPoint<>& entrypoint) { return entrypoint.get_locals(); }),
            "globals",
            sol::readonly_property([](const EntryPoint<>& entrypoint) { return entrypoint.get_globals(); }),
            "meas",
            sol::readonly_property([](const EntryPoint<>& entrypoint) { return entrypoint.get_measurement(); }),
            "sigma",
            sol::readonly_property([](const EntryPoint<>& entrypoint) { return entrypoint.get_sigma(); }));
    }

    void LuaConnector::setup_lua_warn_msg()
    {
        auto lua_warn = [](void* user_data, const char* msg, int to_cont) -> void
        {
            auto& str_stream = *(static_cast<std::string*>(user_data));

            str_stream += msg;

            if (to_cont == 0)
            {
                spdlog::warn("Lua: {}", str_stream);
                str_stream.clear();
            }
        };
        lua_setwarnf(lua_state_.lua_state(), lua_warn, &warn_msg_stream_);
    }
} // namespace centipede::cli
