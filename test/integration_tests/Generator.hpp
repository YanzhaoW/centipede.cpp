#include "centipede/data/ValueError.hpp"
#include "centipede/data/entrypoint.hpp"
#include "centipede/util/common_definitions.hpp"
#include "centipede/util/return_types.hpp"
#include "centipede/writer/binary.hpp"
#include "centipede/writer/capnproto.hpp"
#include <concepts>
#include <cstddef>
#include <format>
#include <glaze/core/opts.hpp>
#include <glaze/json/write.hpp>
#include <map>
#include <mps/MPS.hpp>
#include <mps/data/MilleData.hpp>
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace centipede::test
{
    class Generator
    {
      public:
        struct Config
        {
            bool has_json_output = false;
            common::IO output_format = common::IO::binary;
            std::string_view par_filename;
            std::string_view output_data_filename;
            std::size_t n_events = 0;
        };

        struct Pars
        {
            std::map<std::string, std::vector<double>> init_pars;
            std::map<std::string, std::vector<double>> true_pars;
        };

        explicit Generator(const Config& config)
            : config_{ config }
        {
            switch (config.output_format)
            {
                using enum common::IO;
                case proto:
                {
                    const auto filename = std::format("{}.proto", config.output_data_filename);
                    spdlog::info("Set up proto writer with output file {:?}", filename);
                    binary_writer_.emplace<writer::Capnproto>(writer::Capnproto::Config{ .out_filename = filename });
                    break;
                }
                case binary:
                {
                    const auto filename = std::format("{}.bin", config.output_data_filename);
                    spdlog::info("Set up binary writer with output file {:?}", filename);
                    binary_writer_.emplace<writer::Binary>(writer::Binary::Config{ .out_filename = filename });
                    break;
                }
                case none:
                default:
            }
        }

        auto init(auto& mps) -> VoidError
        {
            pars_.true_pars = mps.get_true_pars_t();
            pars_.init_pars = mps.get_init_pars_t();

            auto is_ok = binary_writer_.visit(
                []<typename T>(T& writer) -> VoidError
                {
                    if constexpr (not std::same_as<T, std::monostate>)
                    {
                        return writer.init();
                    }
                    return {};
                });

            if (not is_ok)
            {
                return is_ok;
            }
            return {};
        }

        void generate_entry(auto& mps,
                            centipede::EntryPoint<>& entry_point_input,
                            const mps::MPSConfig& config,
                            auto callback,
                            bool write_to_writer = true)
        {
            auto entry_span = mps.generate_one_entry_data();

            const auto& data = mps.get_event_data();
            if (data.size() < config.detector.spec.num_modules / 2)
            {
                return;
            }

            for (const auto& entrypoint : entry_span)
            {
                entry_point_input.reset();
                entry_point_input.set_measurement(ValueError<float>{ 0.F, 0.F })
                    .set_globals(std::views::zip_transform(
                        [](const auto& globals, const auto& errs) -> auto
                        { return std::pair{ globals.first - 1, ValueError{ globals.second, errs.second } }; },
                        entrypoint.globals,
                        entrypoint.global_errs))
                    .set_locals(std::views::zip_transform([](const auto& locals, const auto& errs) -> auto
                                                          { return ValueError{ locals, errs }; },
                                                          entrypoint.locals,
                                                          entrypoint.local_errs));
                spdlog::trace("entrypoint: {}", entry_point_input);

                if (write_to_writer)
                {
                    // [[maybe_unused]] auto is_ok = binary_writer_.add_entrypoint(entry_point_input);
                    [[maybe_unused]] auto is_ok = binary_writer_.visit(
                        [&entry_point_input]<typename T>(T& writer) -> VoidError
                        {
                            if constexpr (not std::same_as<T, std::monostate>)
                            {
                                return writer.add_entrypoint(entry_point_input);
                            }
                            return {};
                        });
                }
                callback(entrypoint);
            }

            [[maybe_unused]] auto is_ok = binary_writer_.visit(
                []<typename T>(T& writer) -> ResultError<std::size_t>
                {
                    if constexpr (not std::same_as<T, std::monostate>)
                    {
                        return writer.write_current_entry();
                    }
                    return 0;
                });
        }

        auto generate(auto& mps, const mps::MPSConfig& config)
        {
            auto entry_point_input = centipede::EntryPoint{};
            spdlog::info("Starting generating simulation data of {} events.", config_.n_events);

            for (auto _ : std::views::iota(0UZ, config_.n_events))
            {
                generate_entry(mps, entry_point_input, config, [](const mps::MilleEntryPoint&) -> void {});
            }
            save_pars_to_file(pars_, config_.par_filename);
        }

      private:
        Config config_;
        Pars pars_;
        using OutputTypes = std::variant<std::monostate, writer::Binary, writer::Capnproto>;
        OutputTypes binary_writer_;

        template <typename T>
        void save_pars_to_file(const T& data, std::string_view filename)
        {
            [[maybe_unused]] auto has_error =
                glz::write_file_json<glz::opts{ .minified = true }>(data, filename, std::string{});
        }
    };

} // namespace centipede::test
