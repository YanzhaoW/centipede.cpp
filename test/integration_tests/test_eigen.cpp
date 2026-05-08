#include "Simulator.hpp"
#include "centipede/centipede.hpp"
#include "centipede/util/eigen_formatter.hpp" // IWYU pragma: keep
#include <Eigen/Core>
#include <cstdio>
#include <cstdlib>
#include <cxxopts.hpp>
#include <format>
#include <glaze/core/opts.hpp>
#include <glaze/glaze.hpp>
#include <glaze/json/write.hpp>
#include <ios>
#include <istream>
#include <magic_enum/magic_enum.hpp>
#include <map>
#include <mps/MPS.hpp>
#include <mps/detector_utils/DetectorTypes.hpp>
#include <mps/utils/CommonAlias.hpp>
#include <optional>
#include <print>
#include <ranges>
#include <set>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    enum class ResStatus
    {
        succeed,
        fail,
    };
} // namespace

constexpr auto par_filename = std::string_view{ "pars.json" };
constexpr auto output_data_filename = std::string_view{ "mille_data.bin" };

namespace spdlog::level
{
    auto operator>>(std::istream& in, level_enum& level) -> std::istream&
    {
        std::string level_str;
        in >> level_str;

        auto input_level_enum = magic_enum::enum_cast<spdlog::level::level_enum>(level_str);
        if (not input_level_enum)
        {
            spdlog::error(
                "Unknown log level {:?}. Available values: {}", level_str, magic_enum::enum_names<level_enum>());
            in.setstate(std::ios::failbit);
        }
        else
        {
            level = input_level_enum.value();
        }

        return in;
    }
} // namespace spdlog::level

auto main(int argc, char** argv) -> int
{

    auto app = cxxopts::Options{ "Testing with simulated data from MPS" };

    auto has_json_output = false;
    auto has_mille_output = false;
    auto n_events = 1000UZ;
    auto n_runs = 1UZ;
    auto log_level = spdlog::level::info;

    app.add_options()("h,help", "Print usage");

    app.add_options()(
        "json-output", "Output event data to a json file", cxxopts::value(has_json_output)->default_value("false"))(
        "mille-output", "Output event data to a mille file", cxxopts::value(has_mille_output)->default_value("false"))(
        "r,n-runs", "Set the number of runs", cxxopts::value(n_runs)->default_value(std::format("{}", n_runs)))(
        "n,n_events", "Set the number of events", cxxopts::value(n_events)->default_value(std::format("{}", n_events)));

    app.add_options()("l,log",
                      std::format("Set the log level among: {}", magic_enum::enum_names<spdlog::level::level_enum>()),
                      cxxopts::value(log_level)->default_value(std::string{ magic_enum::enum_name(log_level) }));

    auto result = app.parse(argc, argv);

    if (result.count("help"))
    {
        std::println("{}", app.help());
        return EXIT_SUCCESS;
    }

    spdlog::set_level(log_level);

    auto config = mps::MPSConfig{};
    config.detector.init_sigma = 0.1;
    config.detector.spec.stddevs["a"] = 0.;
    config.detector.spec.stddevs["b"] = 0.;
    config.detector.spec.module_width = 0.;
    config.detector.spec.parameter_ranges["a"] = MinMaxPair{ .min = 0.9, .max = 1.9 };
    config.detector.spec.parameter_ranges["b"] = MinMaxPair{ .min = 1., .max = 1.5 };
    config.detector.mille_config.fixed_parameters.at("a") = std::vector{ 0uz };
    config.detector.mille_config.fixed_parameters.at("b") = std::vector<std::size_t>{ 0uz, 20uz };

    auto mps = mps::MPS<mps::DetectorType::generalized>{ config };
    mps.init();

    auto simulator = centipede::test::Simulator{ {
        .has_json_output = has_json_output,
        .has_mille_output = has_mille_output,
        .par_filename = par_filename,
        .output_data_filename = output_data_filename,
        .n_events = n_events,
    } };

    {
        [[maybe_unused]] auto is_ok = simulator.init(mps);
    }

    const auto n_globals = config.detector.spec.num_modules * 2;
    auto handler = centipede::Handler<float, { .engine_type = centipede::MatrixEngine::eigen }>{
        { .n_globals = n_globals, .fixed_parameter_ids = std::set{ 0UZ, 1UZ, 41UZ } }
    };

    const auto& master_engine = handler.get_engine();
    spdlog::debug("par id map: {}", master_engine.get_par_id_map().get_unfixed_par_id_map());

    for (const auto _ : std::views::iota(0UZ, n_runs))
    {
        auto is_ok = simulator.run_once(mps, handler, config);
        if (not is_ok)
        {
            return EXIT_FAILURE;
        }
    }

    simulator.save_output(config);

    simulator.print_result(handler);

    return EXIT_SUCCESS;
}
