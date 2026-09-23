#pragma once

#include "centipede/data/ValueError.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/return_types.hpp"
#include "centipede/writer/binary.hpp"
#include <algorithm>
#include <cstddef>
#include <glaze/core/opts.hpp>
#include <glaze/json/write.hpp>
#include <iterator>
#include <map>
#include <mps/MPS.hpp>
#include <mps/data/EventData.hpp>
#include <mps/data/MilleData.hpp>
#include <mps/millepede/MillePedeAdaptor.hpp>
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace centipede::test
{
    struct OutputPars
    {
        struct Pars
        {
            std::map<std::string, std::vector<double>> init_pars;
            std::map<std::string, std::vector<double>> init_pars_t;
            std::map<std::string, std::vector<double>> fit_pars_t;
            std::map<std::string, std::vector<double>> fit_pars;
        };
        std::map<std::string, std::vector<double>> true_pars;
        std::map<std::string, std::vector<double>> true_pars_t;
        std::vector<Pars> others;
    };

    class Simulator
    {
      public:
        struct Config
        {
            bool has_json_output = false;
            bool has_mille_output = false;
            std::string_view par_filename;
            std::string_view output_data_filename;
            std::size_t n_events = 0;
        };

        Simulator(const Config& config)
            : config_{ config }
            , binary_writer_{ { .out_filename = std::string{ config.output_data_filename } } }
        {
        }

        auto init(auto& mps) -> VoidError
        {
            output_pars_.true_pars_t = mps.get_true_pars_t();
            output_pars_.true_pars = mps.get_true_pars();

            if (config_.has_mille_output)
            {
                auto is_ok = binary_writer_.init();
                if (not is_ok)
                {
                    return is_ok;
                }
            }
            return {};
        }

        auto run_once(auto& mps, auto& handler, const mps::MPSConfig& config, std::size_t run_idx = 0) -> bool
        {

            spdlog::info("Starting centipede algorithm with {} events.", config_.n_events);
            auto entry_point_input = centipede::EntryPoint{};
            reset();

            if (not output_pars_.others.empty())
            {
                mps.set_init_t_pars(output_pars_.others.back().fit_pars_t);
            }

            for (auto _ : std::views::iota(0UZ, config_.n_events))
            {
                auto entry_span = mps.generate_one_entry_data();

                const auto& data = mps.get_event_data();
                const auto& local_fits = mps.get_local_fits();
                const auto& local_fit_values = mps.get_local_fit_values();

                if (data.size() < config.detector.spec.num_modules / 2)
                {
                    continue;
                }

                if (config_.has_json_output)
                {
                    sim_data.push_back(data);
                    local_fit_data.push_back(local_fit_values);
                    auto& entry = entries.emplace_back();
                    entry.data.reserve(entry_span.size());
                    std::ranges::copy(entry_span, std::back_inserter(entry.data));
                    entry.local_fits = local_fits;
                }

                for (const auto& entrypoint : entry_span)
                {
                    entry_point_input.reset();
                    entry_point_input.set_measurement(ValueError<float>{ entrypoint.measurement, entrypoint.sigma })
                        .set_globals(entrypoint.globals |
                                     std::views::transform([](const auto& globals)
                                                           { return std::pair{ globals.first - 1, globals.second }; }))
                        .set_locals(entrypoint.locals);

                    if (run_idx == 0 and config_.has_mille_output)
                    {
                        [[maybe_unused]] auto is_ok = binary_writer_.add_entrypoint(entry_point_input);
                    }

                    auto res = handler.add_entrypoint(entry_point_input);
                    if (not res)
                    {
                        spdlog::error("Error from adding the current point: {}", res.error());
                    }
                }
                if (run_idx == 0 and config_.has_mille_output)
                {
                    [[maybe_unused]] auto is_ok = binary_writer_.write_current_entry();
                }
                auto res = handler.analyze_current_entry();
                const auto& state = handler.get_slave_entry_state();

                const auto& engine = handler.get_slave_engine();
                const auto& buffers = engine.get_buffers();

                spdlog::trace("global factor mat: {}", engine.get_global_factor_matrix());
                spdlog::trace("local solution: {}", engine.get_local_solutions());
                spdlog::trace("global local weight: {}", buffers.global_local_weighted_t.toDense().eval());
                spdlog::trace("local cov: {}", buffers.local_weighted_square_inv);
                spdlog::trace("global weight: {}", buffers.global_weighted_square.toDense().eval());
                spdlog::trace("global update: {}", buffers.global_square_update.toDense().eval());
                spdlog::trace("global rhs vec: {}", engine.get_global_rhs_vector());

                if (not res)
                {
                    if (res.error() != centipede::ErrorCode::analysis_empty_entry)
                    {
                        spdlog::error("Error from analyzing the current entry: {}", res.error());
                        spdlog::info("p-value: {}, chi2: {}", state.p_value, state.chi2);
                    }
                }
            }

            if (not solve(handler))
            {
                return false;
            }
            add_pars(mps, handler);

            return true;
        }

        void set_json_output(bool val = true) { config_.has_json_output = val; }
        void set_mille_output(bool val = true) { config_.has_mille_output = val; }

        void reset()
        {
            sim_data.clear();
            entries.clear();
            local_fit_data.clear();
        }

        void save_output(const mps::MPSConfig& config)
        {
            if (config_.has_json_output)
            {
                save_config_to_file(config, "config.json");
                save_data_to_file(sim_data, "data.json");
                save_data_to_file(entries, "entry.json");
                save_data_to_file(local_fit_data, "fit_data.json");
            }
            spdlog::info("Fitting processes finished. Parameters are written to the file {:?}.", config_.par_filename);

            [[maybe_unused]] auto ec = glz::write_file_json(output_pars_, config_.par_filename, std::string{});
        }

        void print_result(const auto& handler)
        {

            const auto& result = handler.get_result();
            spdlog::debug("eigen values: {}", result.eigen_values);
            spdlog::info("result: \n\t{}", result);
            spdlog::debug("par corrections: \n\t{}", result.parameters);
            const auto& pars = output_pars_.others.back();
            for (const auto [init_par_vals,
                             true_par_vals,
                             fit_par_vals,
                             init_par_vals_t,
                             true_par_vals_t,
                             fit_par_vals_t] : std::views::zip(pars.init_pars,
                                                               output_pars_.true_pars,
                                                               pars.fit_pars,
                                                               pars.init_pars_t,
                                                               output_pars_.true_pars_t,
                                                               pars.fit_pars_t))
            {
                spdlog::debug("parameter {:?}:", init_par_vals.first);
                spdlog::debug("index\tinit\ttrue\tfit \t| init_t\ttrue_t\tfit_t");
                for (const auto [idx,
                                 init_par_val,
                                 true_par_val,
                                 fit_par_val,
                                 init_par_val_t,
                                 true_par_val_t,
                                 fit_par_val_t] : std::views::zip(std::views::iota(0UZ),
                                                                  init_par_vals.second,
                                                                  true_par_vals.second,
                                                                  fit_par_vals.second,
                                                                  init_par_vals_t.second,
                                                                  true_par_vals_t.second,
                                                                  fit_par_vals_t.second))
                {
                    spdlog::debug("{}\t{:.3f}\t{:.3f}\t{:.3f} \t| {:.3f}\t\t{:.3f}\t{:.3f}",
                                  idx,
                                  init_par_val,
                                  true_par_val,
                                  fit_par_val,
                                  init_par_val_t,
                                  true_par_val_t,
                                  fit_par_val_t);
                }
            }
            // spdlog::debug("fit parameters: {}", pars.fit_pars);
            // spdlog::debug("true parameters: {}", pars.true_pars);
            // spdlog::debug("init parameters: {}", pars.init_pars);
        }

      private:
        struct EntryData
        {
            std::vector<double> local_fits;
            std::vector<mps::MilleEntryPoint> data;
        };

        Config config_;

        std::string par_filename_;
        OutputPars output_pars_;
        writer::Binary binary_writer_;

        std::vector<mps::Data> sim_data{};
        std::vector<EntryData> entries{};
        std::vector<mps::MillePedeAdaptor::FitInput> local_fit_data{};

        void add_pars(const auto& mps, const auto& handler)
        {
            const auto& result = handler.get_result();
            auto& pars = output_pars_.others.emplace_back();

            pars.init_pars = mps.get_init_pars();
            pars.init_pars_t = mps.get_init_pars_t();

            auto fit_par_error_t = std::map<std::string, std::vector<double>>{};

            for (const auto [idx, key_val] : std::views::zip(std::views::iota(0UZ), pars.init_pars_t))
            {
                auto& val = pars.fit_pars_t.try_emplace(key_val.first, std::vector<double>{}).first->second;
                auto& errs = fit_par_error_t.try_emplace(key_val.first, std::vector<double>{}).first->second;
                val.clear();
                val.reserve(key_val.second.size());
                errs.resize(key_val.second.size());
                for (const auto [par_idx, par_val] : std::views::zip(std::views::iota(0UZ), key_val.second))
                {
                    auto iter = result.parameters.find(idx + par_idx * 2);
                    if (iter == result.parameters.end())
                    {
                        val.push_back(par_val);
                    }
                    else
                    {
                        val.push_back(par_val + iter->second);
                    }
                }
            }
            pars.fit_pars = mps.get_inverse_transform(std::tie(pars.fit_pars_t, fit_par_error_t)).first;
        }

        auto solve(auto& handler) -> bool
        {
            spdlog::info("Solving the equations");
            auto is_ok = handler.solve();

            if (not is_ok)
            {
                spdlog::error("Error from the solving: {}", is_ok.error());
                return false;
            }
            return true;
        }

        template <typename T>
        void save_data_to_file(const std::vector<T>& data, std::string_view filename)
        {
            [[maybe_unused]] auto ec =
                glz::write_file_json<glz::opts{ .minified = true }>(data, filename, std::string{});
        }

        void save_config_to_file(const auto& data, std::string_view filename)
        {
            [[maybe_unused]] auto ec =
                glz::write_file_json<glz::opts{ .minified = true }>(data, filename, std::string{});
        }
    };
} // namespace centipede::test
