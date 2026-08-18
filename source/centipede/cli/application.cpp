#include "application.hpp"
#include "centipede/centipede.hpp"
#include "centipede/cli/lua_connector.hpp"
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <format>
#include <glaze/core/context.hpp>
#include <glaze/core/opts.hpp>
#include <glaze/core/reflect.hpp>
#include <glaze/csv/read.hpp>
#include <glaze/csv/write.hpp>
#include <glaze/json/generic_fwd.hpp>
#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <map>
#include <ranges>
#include <sol/error.hpp>
#include <span>
#include <spdlog/spdlog.h>
#include <string>
#include <system_error>
#include <vector>

namespace centipede
{
    namespace cli
    {
        /**
         * @brief Struct for one line in the parameter output.
         */
        struct ParResultRow
        {
            std::size_t par_id;
            float init;
            float value;
            float sigma;
            float correction;
            std::size_t run_id;
        };

        /**
         * @brief Struct for parameter output.
         */
        struct ParResult
        {
            std::vector<std::size_t> par_id;
            std::vector<float> init;
            std::vector<float> value;
            std::vector<float> sigma;
            std::vector<float> correction;
            std::vector<std::size_t> run_id;

            void reserve(std::size_t size)
            {
                par_id.reserve(size);
                init.reserve(size);
                value.reserve(size);
                sigma.reserve(size);
                correction.reserve(size);
                run_id.reserve(size);
            }

            void push_back(const ParResultRow& row)
            {
                par_id.push_back(row.par_id);
                init.push_back(row.init);
                value.push_back(row.value);
                sigma.push_back(row.sigma);
                correction.push_back(row.correction);
                run_id.push_back(row.run_id);
            }
        };

        /**
         * @brief Custom JSON config.
         */
        struct JsonWriteOpt : glz::opts
        {
            bool new_lines_in_arrays = false;
            uint8_t indentation_width = 2;
        };
    } // namespace cli

    namespace
    {
        auto read_id_values_from_json(std::vector<std::size_t>& ids,
                                      std::vector<float>& values,
                                      const cli::Config::Input::InitPar& config) -> VoidStr
        {
            auto json_obj = glz::generic{};
            auto str_buffer = std::string{};
            auto ec = glz::read_file_json(json_obj, config.filename, str_buffer);
            if (ec)
            {
                return std::unexpected{ glz::format_error(ec, str_buffer) };
            }
            if (not json_obj.is_object())
            {
                return std::unexpected{ "Parameter initial value file does not contain the valid JSON format!" };
            }

            if (not json_obj.contains(config.id))
            {
                return std::unexpected{ std::format(
                    "Parameter initial value (JSON) file does not contain {:?} field for parameter IDs!", config.id) };
            }

            if (not json_obj.contains(config.value))
            {
                return std::unexpected{ std::format(
                    "Parameter initial value file (JSON) does not contain {:?} field for parameter initial values!",
                    config.value) };
            }

            ids = json_obj[config.id].as<std::vector<std::size_t>>();
            values = json_obj[config.value].as<std::vector<float>>();

            return {};
        }

        auto read_id_values_from_csv(std::vector<std::size_t>& ids,
                                     std::vector<float>& values,
                                     const cli::Config::Input::InitPar& config) -> VoidStr
        {
            auto table = std::map<std::string, std::vector<std::string>>{};
            auto str_buffer = std::string{};
            ids.clear();
            values.clear();
            auto ec = glz::read_file_csv<glz::colwise>(table, config.filename, str_buffer);
            if (ec)
            {
                return std::unexpected{ glz::format_error(ec, str_buffer) };
            }

            if (not table.contains(config.id))
            {
                return std::unexpected{ std::format(
                    "Parameter initial value (csv) file does not contain {:?} column for parameter IDs!", config.id) };
            }

            if (not table.contains(config.value))
            {
                return std::unexpected{ std::format(
                    "Parameter initial value file (csv) does not contain {:?} column for parameter initial values!",
                    config.value) };
            }

            const auto& id_strs = table.at(config.id);
            ids.reserve(id_strs.size());
            for (const auto& id_str : id_strs)
            {
                auto value = std::size_t{};
                auto error = std::from_chars(id_str.data(), id_str.data() + id_str.size(), value).ec;
                if (error != std::errc{})
                {
                    return std::unexpected{ std::format(
                        "Error occurred while parsing string {:?} to integers for parameter IDs due to {:?}",
                        id_str,
                        std::make_error_code(error).message()) };
                }
                ids.push_back(value);
            }

            const auto& value_strs = table.at(config.value);
            values.reserve(id_strs.size());
            for (const auto& value_str : value_strs)
            {
                auto value = float{};
                auto error = std::from_chars(value_str.data(), value_str.data() + value_str.size(), value).ec;
                if (error != std::errc{})
                {
                    return std::unexpected{ std::format(
                        "Error occurred while parsing string {:?} to floats for parameter initial value due to {:?}",
                        value_str,
                        std::make_error_code(error).message()) };
                }
                values.push_back(value);
            }

            return {};
        }
    } // namespace

    auto Application::set_global_initial_values_from_file(const cli::Config::Input::InitPar& init_config) -> VoidStr
    {
        if (init_config.filename.empty() or init_config.filename == "")
        {
            spdlog::debug("Parameter initial value file is absent. Setting all values to be zeros.");
            return {};
        }
        if (init_config.id.empty() or init_config.id == "")
        {
            return std::unexpected{ "Id column name must be specifed to read parameter initial values!" };
        }
        if (init_config.value.empty() or init_config.value == "")
        {
            return std::unexpected{ "Value column name must be specifed to read parameter initial values!" };
        }

        return handle_->visit(
            [this, &init_config](auto& handle) -> VoidStr
            {
                auto ids = std::vector<std::size_t>{};
                auto init_values = std::vector<float>{};

                if (init_config.filename.ends_with(".json"))
                {
                    auto is_ok = read_id_values_from_json(ids, init_values, init_config);
                    if (not is_ok)
                    {
                        return is_ok;
                    }
                }
                else if (init_config.filename.ends_with(".csv"))
                {
                    auto is_ok = read_id_values_from_csv(ids, init_values, init_config);
                    if (not is_ok)
                    {
                        return is_ok;
                    }
                }
                else
                {
                    return std::unexpected{ std::format(
                        "Cannot read the initial parameters due to the unsupported file extension from the file {:?}",
                        init_config.filename) };
                }

                spdlog::info(std::format("Setting the initial values of global parameters from file {:?}",
                                         init_config.filename));
                for (const auto& [id, init_value] : std::views::zip(ids, init_values))
                {
                    [[maybe_unused]] auto is_ok = handle.set_global_init_value(id, init_value);
                }

                return {};
            });
    }

    auto Application::init() -> VoidStr
    {
        reader_ = reader::Binary{ { .in_filename = config_.input.data_filename } };
        spdlog::debug("Use the configuration:\n{}", config_.engine);

        results_.reserve(config_.num_of_runs);

        return centipede::create<float, { .engine_type = MatrixEngine::eigen }>(config_.engine)
            .and_then(
                [this](auto handle) -> VoidStr
                {
                    handle_ = std::make_unique<Handlers>(std::move(handle));
                    return set_global_initial_values_from_file(config_.input.init_par);
                })
            .and_then([this] -> VoidStr
                      { return reader_.visit([](auto& reader) -> VoidStr { return reader.init(); }); });
    }

    auto Application::use_config(const std::string& filename) -> VoidStr
    {

        return lua_connector_.init(config_)
            .and_then([this, &filename] -> VoidStr { return lua_connector_.read_user_config(filename, config_); })
            .transform([] -> void { spdlog::info("Lua file is read successfully!"); });

        return {};
    }

    auto Application::run_once(std::size_t run_idx, auto& reader, auto& handle) -> VoidStr
    {
        spdlog::info("Run {} starts ...", run_idx);
        const auto& master_engine = handle.get_master_engine();
        for (const auto& entry : reader)
        {
            for (const auto& entrypoint : entry)
            {

                if (config_.hooks.post_entrypoint_read)
                {
                    // std::println("passing here .... ");
                    auto res = config_.hooks.post_entrypoint_read.value()(&entrypoint);
                    if (not res.valid())
                    {
                        return std::unexpected{ std::format("{}", res.template get<sol::error>().what()) };
                    }
                }

                auto is_ok = handle.add_entrypoint(entrypoint);
                if (not is_ok)
                {
                    return std::unexpected{ std::format("{}", is_ok.error()) };
                }
            }
            auto is_ok = handle.analyze_current_entry();
            if (not is_ok)
            {
                return std::unexpected{ std::format("{}", is_ok.error()) };
            }
        }
        auto& result = results_.emplace_back();
        auto is_ok = handle.solve(result);
        spdlog::info("Run {} finished with \n{}\n Starting to solve global matrices", run_idx, master_engine.get_log());
        if (not is_ok)
        {
            return std::unexpected{ std::format("{}", is_ok.error()) };
        }
        return {};
    }

    auto Application::save_output() -> VoidStr
    {
        auto results_span = config_.output.only_last_run ? std::span(results_.begin(), 1) : std::span(results_);

        auto par_results = cli::ParResult{};

        par_results.reserve((config_.output.only_last_run ? 1 : config_.num_of_runs) + config_.engine.n_globals);
        const auto& centipede_config = handle_->visit([](auto& handle) { return handle.get_config(); });
        const auto& global_init_values = centipede_config.global_init_values;

        for (const auto [run_id, result] : std::views::zip(std::views::iota(0UZ), results_))
        {
            if (config_.output.only_last_run and (run_id != (config_.num_of_runs - 1)))
            {
                continue;
            }

            for (const auto par_id : std::views::iota(0UZ, config_.engine.n_globals))
            {
                auto par_val_iter = result.parameters.find(par_id);
                auto init_value = global_init_values.at(par_id);
                if (par_val_iter == result.parameters.end())
                {
                    par_results.push_back({ .par_id = par_id,
                                            .init = init_value,
                                            .value = init_value,
                                            .sigma = -1.F,
                                            .correction = 0.F,
                                            .run_id = run_id });
                }
                else
                {
                    par_results.push_back({ .par_id = par_id,
                                            .init = init_value,
                                            .value = init_value + par_val_iter->second,
                                            .sigma = 0.F,
                                            .correction = par_val_iter->second,
                                            .run_id = run_id });
                }
            }
        }
        auto str_buffer = std::string{};
        auto err = glz::error_ctx{};

        if (config_.output.par_filename.ends_with(".json"))
        {
            err = glz::write_file_json<cli::JsonWriteOpt{ { .prettify = true } }>(
                par_results, config_.output.par_filename, str_buffer);
        }
        else if (config_.output.par_filename.ends_with(".csv"))
        {
            err = glz::write_file_csv<glz::colwise>(par_results, config_.output.par_filename, str_buffer);
        }
        else
        {
            return std::unexpected{ std::format(
                "Parameter writing does not support the file extension from the filename {:?}!",
                config_.output.par_filename) };
        }
        if (err)
        {
            return std::unexpected{ glz::format_error(err, str_buffer) };
        }

        return {};
    }

    void Application::set_global_initial_values(auto& handle)
    {
        const auto& result = handle.get_result();
        const auto& global_init_values = handle.get_config().global_init_values;
        handle.set_global_init_values(global_init_values);
    }

    auto Application::run() -> VoidStr
    {
        spdlog::info("Starting the data analysis with {} run(s)", config_.num_of_runs);
        return reader_.visit(
            [this](auto& reader) -> auto
            {
                return handle_->visit(
                    [this, &reader](auto& handle) -> VoidStr
                    {
                        for (const auto idx : std::views::iota(0UZ, config_.num_of_runs))
                        {
                            if (idx != 0)
                            {
                                set_global_initial_values(handle);
                            }
                            if (auto is_ok = run_once(idx, reader, handle); not is_ok)
                            {
                                return is_ok;
                            }
                        }
                        return {};
                    });
            });
    }

} // namespace centipede
