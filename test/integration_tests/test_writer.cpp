#include "centipede/data/entry.hpp"
#include "centipede/writer/binary.hpp"
#include <algorithm>
#include <cstdio> // IWYU pragma: keep
#include <cstdlib>
#include <print>
#include <random>
#include <ranges>

constexpr auto N_ENTRIES = 4000;
constexpr auto MAX_N_ENTRYPOINTS = 20;
constexpr auto N_LOCALS = 3;
constexpr auto N_GLOBALS = 4;
constexpr auto MIN_VALUE = 1.F;
constexpr auto MAX_VALUE = 10.F;

auto main() -> int
{
    auto writer = centipede::writer::Binary{};

    auto init_err = writer.init();

    if (not init_err.has_value())
    {
        std::println(stderr, "Error: {}", init_err.error());
        return EXIT_FAILURE;
    }

    using NewEntryPoint = centipede::EntryPoint<N_LOCALS, N_GLOBALS>;
    auto entry_point = NewEntryPoint{};

    auto rnd_dev = std::random_device{};
    auto rnd_engine = std::mt19937{ rnd_dev() };
    auto rnd_float_dst = std::uniform_real_distribution<float>{ MIN_VALUE, MAX_VALUE };
    auto rnd_int_dst = std::uniform_int_distribution<int>{ 1, MAX_N_ENTRYPOINTS };
    auto generate_int_rnd = [&rnd_int_dst, &rnd_engine]() -> int { return rnd_int_dst(rnd_engine); };
    auto generate_float_rnd = [&rnd_float_dst, &rnd_engine]() -> float { return rnd_float_dst(rnd_engine); };
    auto randomize_locals = [&generate_float_rnd](NewEntryPoint::LocalDerivs& local_derivs) -> void
    { std::ranges::generate(local_derivs, generate_float_rnd); };
    auto randomize_globals = [&generate_float_rnd,
                              &generate_int_rnd](NewEntryPoint::GlobalDerivs& global_derivs) -> void
    {
        std::ranges::generate(global_derivs | std::views::values, generate_float_rnd);
        std::ranges::generate(global_derivs | std::views::keys, generate_int_rnd);
    };

    for ([[maybe_unused]] auto entry_idx : std::views::iota(0, N_ENTRIES))
    {
        auto n_entrypoints = generate_int_rnd();
        for ([[maybe_unused]] auto entrypoint_idx : std::views::iota(0, n_entrypoints))
        {
            randomize_locals(entry_point.local_derivs);
            randomize_globals(entry_point.global_derivs);
            entry_point.measurement = generate_float_rnd();
            entry_point.sigma = generate_float_rnd();
            auto err = writer.add_entrypoint(entry_point);
            if (not err.has_value())
            {
                std::println(stderr, "Error: {}", err.error());
                return EXIT_FAILURE;
            }
        }
        [[maybe_unused]] auto size = writer.write_current_entry();
    }

    return EXIT_SUCCESS;
}
