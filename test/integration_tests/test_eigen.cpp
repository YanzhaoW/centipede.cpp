#include "centipede/centipede.hpp"
#include "centipede/data/entrypoint.hpp"
#include <cstdlib>
#include <mps/MPS.hpp>
#include <mps/detector_utils/DetectorTypes.hpp>
#include <print>
#include <ranges>
#include <vector>

namespace
{
    // void entrypoints_generator() {}
} // namespace

auto main() -> int
{
    const auto config = mps::MPSConfig{};

    auto mps = mps::MPS<mps::DetectorType::generalized>{ config };
    mps.init();

    auto handler = centipede::Handler<float, { .engine_type = centipede::MatrixEngine::eigen }>{};

    auto entry_point_input = centipede::EntryPoint{};
    for (auto _ : std::views::iota(0, 1000))
    {
        auto entry_span = mps.generate_one_entry_data();

        for (const auto& entrypoint : entry_span)
        {
            entry_point_input.reset();
            entry_point_input.set_measurement(entrypoint.measurement)
                .set_sigma(entrypoint.sigma)
                .set_globals(entrypoint.globals)
                .set_locals(entrypoint.locals);
        }
        auto res = handler.analyze_current_entry();
        if (not res)
        {
            std::println("Error: {}", res.error());
            return EXIT_FAILURE;
        }
    }

    auto is_ok = handler.solve();

    if (not is_ok)
    {
        std::println("Error: {}", is_ok.error());
        return EXIT_FAILURE;
    }

    const auto& result = handler.get_result();

    std::println("{}", result);

    return EXIT_SUCCESS;
}
