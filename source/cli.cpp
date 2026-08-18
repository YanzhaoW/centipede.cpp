#include "centipede/cli/application.hpp"
#include "centipede/cli/spdlog_stream.hpp" // IWYU pragma: keep
#include "centipede/util/return_types.hpp"
#include <cstdio>
#include <cstdlib>
#include <cxxopts.hpp>
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>

using centipede::VoidStr;

auto main(int argc, char** argv) -> int
{
    auto args = cxxopts::Options{ "centipede", "Command line interface of centipede project." };

    auto log_level = spdlog::level::info;
    auto input_filename = std::string{};
    auto output_par_filename = std::string{};
    auto config_filename = std::string{};

    args.add_options()("h,help", "Print usage");
    args.add_options()("l,log",
                       std::format("Set the log level among: {}", magic_enum::enum_names<spdlog::level::level_enum>()),
                       cxxopts::value(log_level)->default_value(std::string{ magic_enum::enum_name(log_level) }));
    args.add_options()(
        "c,config", "Configuration lua file to be used.", cxxopts::value(config_filename)->default_value("config.lua"));

    auto result = args.parse(argc, argv);

    if (result.contains("help"))
    {
        std::println("{}", args.help());
        return EXIT_SUCCESS;
    }
    if (not result.contains("config"))
    {
        std::println(stderr, "Error: -c or --config is required!");
        return EXIT_FAILURE;
    }

    spdlog::set_level(log_level);

    auto app = centipede::Application{ { .input = { .data_filename = input_filename },
                                         .output = { .par_filename = output_par_filename } } };

    auto is_ok = app.use_config(config_filename)
                     .and_then([&app] -> VoidStr { return app.init(); })
                     .and_then([&app] -> VoidStr { return app.run(); })
                     .and_then([&app] -> VoidStr { return app.save_output(); });

    if (not is_ok)
    {
        spdlog::critical("Error: {}", is_ok.error());
        return EXIT_FAILURE;
    }

    spdlog::info("Application exits successfully!");
    return EXIT_SUCCESS;
}
