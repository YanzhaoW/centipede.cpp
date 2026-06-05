#include "centipede/centipede.hpp"
#include "centipede/data/entrypoint.hpp"
#include <Eigen/Core>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <mps/MPS.hpp>
#include <mps/detector_utils/DetectorTypes.hpp>
#include <print>
#include <ranges>
#include <utility>

namespace
{
    // void entrypoints_generator() {}
} // namespace

auto main() -> int
{
    const auto config = mps::MPSConfig{};

    auto mps = mps::MPS<mps::DetectorType::generalized>{ config };
    mps.init();

    const auto n_globals = config.detector.spec.num_modules * 2;
    auto handler = centipede::Handler<float, { .engine_type = centipede::MatrixEngine::eigen }>{ n_globals };

    auto entry_point_input = centipede::EntryPoint{};
    for (auto _ : std::views::iota(0, 1000))
    {
        auto entry_span = mps.generate_one_entry_data();

        for (const auto& entrypoint : entry_span)
        {
            entry_point_input.reset();
            entry_point_input.set_measurement(entrypoint.measurement)
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

    std::println("{}", result);

    return EXIT_SUCCESS;
}
