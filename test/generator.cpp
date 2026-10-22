#include "integration_tests/Generator.hpp"
#include "centipede/centipede.hpp"
#include "centipede/cli/cxxopts_formatter.hpp" // IWYU pragma: keep
#include <cstdlib>
#include <cxxopts.hpp>
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <mps/MPS.hpp>
#include <mps/detector_utils/DetectorTypes.hpp>
#include <mps/utils/CommonAlias.hpp>
#include <print>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <vector>

constexpr auto par_filename = std::string_view{ "pars.json" };

constexpr auto output_data_filename = std::string_view{ "mille_data" };

auto main(int argc, char** argv) -> int
{

    auto app = cxxopts::Options{ "Generating simulated data using MPS" };
    auto n_events = 1000UZ;
    auto log_level = spdlog::level::info;
    auto output_format = centipede::common::IO::proto;

    app.add_options()("h,help", "Print usage");
    app.add_options()(
        "n,n-events", "Set the number of events", cxxopts::value(n_events)->default_value(std::format("{}", n_events)));

    app.add_options()("l,log",
                      std::format("Set the log level among: {}", magic_enum::enum_names<spdlog::level::level_enum>()),
                      cxxopts::value(log_level)->default_value(std::string{ magic_enum::enum_name(log_level) }));

    app.add_options()(
        "output-format",
        std::format("Set the output format: {}", magic_enum::enum_names<centipede::common::IO>()),
        cxxopts::value(output_format)->default_value(std::string{ magic_enum::enum_name(output_format) }));

    auto result = app.parse(argc, argv);

    if (result.contains("help"))
    {
        std::println("{}", app.help());
        return EXIT_SUCCESS;
    }

    spdlog::set_level(log_level);

    auto config = mps::MPSConfig{};
    config.detector.init_sigma = 0.1;
    config.detector.spec.stddevs["a"] = 0.;
    config.detector.spec.stddevs["b"] = 0.;
    config.detector.spec.module_spacing = 0.05;
    config.detector.spec.module_width = 0.02;
    config.detector.spec.parameter_ranges["a"] = MinMaxPair{ .min = 0.9, .max = 1.9 };
    config.detector.spec.parameter_ranges["b"] = MinMaxPair{ .min = 1., .max = 1.5 };
    config.detector.spec.stddevs["a"] = 0.0;
    config.detector.spec.stddevs["b"] = 0.0;
    config.detector.mille_config.fixed_parameters.at("a") = std::vector{ 0uz };
    config.detector.mille_config.fixed_parameters.at("b") = std::vector<std::size_t>{ 0uz, 20uz };

    auto mps = mps::MPS<mps::DetectorType::generalized>{ config };
    mps.init();

    auto generator = centipede::test::Generator{ {
        .output_format = output_format,
        .par_filename = par_filename,
        .output_data_filename = output_data_filename,
        .n_events = n_events,
    } };

    [[maybe_unused]] auto is_ok = generator.init(mps);

    generator.generate(mps, config);

    return EXIT_SUCCESS;
}
