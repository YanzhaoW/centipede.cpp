#pragma once

#include "centipede/core/config.hpp"
#include <cstddef>
#include <optional>
#include <ranges>
#include <sol/forward.hpp>
#include <sol/state.hpp>
#include <sol/table.hpp>
#include <sol/types.hpp>
#include <spdlog/spdlog.h>
#include <string>

namespace centipede::cli
{
    /**
     * @brief Configuration struct for Application.
     *
     */
    struct Config
    {
        /**
         * @brief Struct for input settings.
         */
        struct Input
        {
            std::string data_filename; //!< Filename for input binary data

            /**
             * @brief Struct for setting initial parameter values.
             */
            struct InitPar
            {
                std::string filename; //!< File name containing the inital values of global parameters.
                std::string id;       //!< Column/field name for par ID.
                std::string value;    //!< Column/field name for init values.
            } init_par{};             //!< Configs for init parameters.
        } input{};                    //!< Output related configs.
        /**
         * @brief Struct for output settings.
         */
        struct Output
        {
            std::string par_filename;      //!< Filename for output parameter file.
            bool only_last_run = true;     //!< Only save the results of the last run.
        } output{};                        //!< Output related configs.
        std::size_t num_of_runs = 1;       //!< Total number of runs.
        std::size_t max_num_of_events = 0; //!< Maximum number of events read from file. 0 means all events.
        centipede::Config<float> engine{}; //!< Engine configurations.

        /**
         * @brief Struct for callback functions.
         */
        struct Hooks
        {
            std::optional<sol::protected_function> post_entrypoint_read; //!< callback after reading one entrypoint.
        } hooks{};                                                       //!< Lua callbacks.
    };

    inline auto create_lua_table(sol::state& lua_state, const Config& config) -> sol::table
    {
        auto lua_table = lua_state.create_table();

        lua_table["input"] = lua_state.create_table();
        lua_table["input"]["data_filename"] = config.input.data_filename;

        lua_table["input"]["init_par"] = lua_state.create_table();
        lua_table["input"]["init_par"]["filename"] = config.input.init_par.filename;
        lua_table["input"]["init_par"]["id"] = config.input.init_par.id;
        lua_table["input"]["init_par"]["value"] = config.input.init_par.value;

        lua_table["output"] = lua_state.create_table();
        lua_table["output"]["par_filename"] = config.output.par_filename;
        lua_table["output"]["only_last_run"] = config.output.only_last_run;

        lua_table["num_of_runs"] = config.num_of_runs;
        lua_table["max_num_of_events"] = config.max_num_of_events;

        lua_table["engine"] = lua_state.create_table();

        lua_table["engine"]["chi2_factor"] = config.engine.chi2_factor;
        lua_table["engine"]["n_globals"] = config.engine.n_globals;
        lua_table["engine"]["alpha"] = config.engine.alpha;

        auto fixed_parameter_ids = lua_state.create_table();
        for (const auto& [idx, par_id] : std::views::zip(std::views::iota(1), config.engine.fixed_parameter_ids))
        {
            fixed_parameter_ids[idx] = par_id;
        }
        lua_table["engine"]["fixed_parameter_ids"] = fixed_parameter_ids;

        lua_table["hooks"] = lua_state.create_table();
        lua_table["hooks"]["post_entrypoint_read"] = sol::lua_nil;

        return lua_table;
    }

    inline auto read_lua_table(const sol::table& lua_table, Config& config)
    {
        config.input.data_filename = lua_table["input"]["data_filename"].get<std::string>();

        config.input.init_par.filename = lua_table["input"]["init_par"]["filename"].get<std::string>();
        config.input.init_par.id = lua_table["input"]["init_par"]["id"].get<std::string>();
        config.input.init_par.value = lua_table["input"]["init_par"]["value"].get<std::string>();

        config.output.par_filename = lua_table["output"]["par_filename"].get<std::string>();
        config.output.only_last_run = lua_table["output"]["only_last_run"].get<bool>();

        config.num_of_runs = lua_table["num_of_runs"].get<std::size_t>();
        config.max_num_of_events = lua_table["max_num_of_events"].get<std::size_t>();

        config.engine.chi2_factor = lua_table["engine"]["chi2_factor"].get<float>();
        config.engine.n_globals = lua_table["engine"]["n_globals"].get<std::size_t>();
        config.engine.alpha = lua_table["engine"]["alpha"].get<float>();

        for (const auto& [key, val] : lua_table["engine"]["fixed_parameter_ids"].get<sol::table>())
        {
            config.engine.fixed_parameter_ids.emplace(val.as<std::size_t>());
        }

        auto post_entrypoint_read = lua_table["hooks"]["post_entrypoint_read"].get<sol::object>();
        if (post_entrypoint_read.valid() and post_entrypoint_read != sol::lua_nil and
            post_entrypoint_read.is<sol::protected_function>())
        {
            spdlog::debug("Hook post_entrypoint_read has been set up.");
            config.hooks.post_entrypoint_read = post_entrypoint_read.as<sol::protected_function>();
        }
    }
} // namespace centipede::cli
