#include "centipede/centipede.hpp"
#include "centipede/data/entrypoint.hpp"
#include <Eigen/Core>
#include <boost/config/detail/suffix.hpp>
#include <boost/histogram.hpp>
#include <boost/histogram/axis/regular.hpp>
#include <boost/histogram/make_histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <mps/MPS.hpp>
#include <mps/detector_utils/DetectorTypes.hpp>
#include <print>
#include <ranges>
#include <sstream>
#include <utility>

namespace
{
    enum class ResStatus
    {
        succeed,
        fail,
    };
} // namespace

namespace bh = boost::histogram;

namespace boost
{
    class source_location;

    BOOST_NORETURN void throw_exception(std::exception const&) { std::abort(); }

    BOOST_NORETURN void throw_exception(std::exception const&, boost::source_location const&) { std::abort(); }

} // namespace boost

auto main() -> int
{
    auto config = mps::MPSConfig{};
    config.detector.init_sigma = 1.;

    auto mps = mps::MPS<mps::DetectorType::generalized>{ config };
    mps.init();

    const auto& init_pars = mps.get_init_pars();
    // const auto& init_pars_errors = mps.get_init_pars_errors();
    const auto& true_pars = mps.get_true_pars();
    // const auto& true_pars_errors = mps.get_true_pars_errors();

    std::println("init pars: {}", init_pars);
    // std::println("init par errors: {}", init_pars_errors);
    std::println("true pars: {}", true_pars);
    // std::println("true par errors: {}", true_pars_errors);

    const auto n_globals = config.detector.spec.num_modules * 2;
    auto handler = centipede::Handler<float, { .engine_type = centipede::MatrixEngine::eigen }>{ n_globals };

    auto hist_residuals = bh::make_histogram(bh::axis::regular{ 20, 0., 17., "chi2" });
    auto hist_pvalues = bh::make_histogram(bh::axis::regular{ 20, 0., 1., "pvalues" });

    auto entry_point_input = centipede::EntryPoint{};
    for (auto _ : std::views::iota(0, 1000))
    {
        auto entry_span = mps.generate_one_entry_data();

        // std::println("---------------------");
        for (const auto& entrypoint : entry_span)
        {
            // std::println("current input entry: {}", entrypoint);
            entry_point_input.reset();
            entry_point_input
                .set_measurement(entrypoint.measurement)
                // entry_point_input.set_measurement(0.)
                .set_sigma(entrypoint.sigma)
                .set_globals(entrypoint.globals |
                             std::views::transform([](const auto& globals)
                                                   { return std::pair{ globals.first - 1, globals.second }; }))
                .set_locals(entrypoint.locals);
            auto res = handler.add_entrypoint(entry_point_input);
            if (not res)
            {
                std::println(stderr, "Error from adding the current point: {}", res.error());
            }
        }
        // std::println("Number of entrypoints in the current entry: {}", handler.get_current_state().next_point_index);
        auto res = handler.analyze_current_entry();
        const auto& state = handler.get_current_entry_state();
        hist_residuals(state.chi2);
        hist_pvalues(state.p_value);
        if (not res)
        {
            if (res.error() != centipede::ErrorCode::analysis_empty_entry)
            {
                std::println(stderr, "Error from analyzing the current entry: {}", res.error());
            }
        }
    }

    std::println("Solving the equations");
    auto is_ok = handler.solve();

    if (not is_ok)
    {
        std::println("Error from the solving: {}", is_ok.error());
        return EXIT_FAILURE;
    }

    const auto& result = handler.get_result();

    auto hist_ostream = std::ostringstream{};
    hist_ostream << hist_residuals << hist_pvalues;

    std::println("{}", hist_ostream.str());
    std::println("{}", result);
    std::println("parameters: {}", result.parameters);
    std::println("eigen values: {}", result.eigen_values);

    return EXIT_SUCCESS;
}
