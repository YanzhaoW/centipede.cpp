#pragma once

#include "centipede/data/entry.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <ranges>
#include <vector>

/**
 * @namespace centipede::test
 * @brief Test components of centipede library.
 *
 */

namespace centipede::test
{
    constexpr auto DEFAULT_N_GLOBALS = 4;
    constexpr auto DEFAULT_N_LOCALS = 4;
    constexpr auto DEFAULT_MAX_GLOBAL_ID = 20;
    constexpr auto MAX_VAL = 10.;
    namespace sv = std::views;
    namespace sr = std::ranges;

    // NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
    inline auto generate_random_entry_points(int n_points,
                                             int n_globals = DEFAULT_N_GLOBALS,
                                             int n_locals = DEFAULT_N_LOCALS)
    {
        static auto rand_dev = std::random_device{};
        static auto engine = std::mt19937{ rand_dev() };
        static auto global_id_gen = std::uniform_int_distribution(0, DEFAULT_MAX_GLOBAL_ID - 1);
        static auto value_gen = std::uniform_real_distribution<double>(1., MAX_VAL);

        auto global_ids = sv::iota(0, DEFAULT_MAX_GLOBAL_ID) | sr::to<std::vector<int>>();

        return std::views::iota(0, n_points) |
               std::views::transform(
                   [&](const auto) -> auto
                   {
                       auto entrypoint = EntryPoint<>{}.set_measurement(value_gen(engine)).set_sigma(value_gen(engine));
                       sr::shuffle(global_ids, engine);
                       for (const auto global_id : global_ids | sv::take(n_globals))
                       {
                           entrypoint.add_global(global_id, value_gen(engine));
                       }
                       for (const auto idx : std::views::iota(0, n_locals))
                       {
                           entrypoint.add_local(value_gen(engine));
                       }
                       return entrypoint;
                   }) |
               std::ranges::to<std::vector<EntryPoint<>>>();
    }

    void EXPECT_TRUE_RES(const auto& result)
    {
        EXPECT_TRUE(result.has_value()) << std::format("Error: {}", result.error());
    }

    void ASSERT_TRUE_RES(const auto& result)
    {
        ASSERT_TRUE(result.has_value()) << std::format("Error: {}", result.error());
    }

    void EXPECT_FALSE_RES(const auto& result)
    {
        EXPECT_FALSE(result.has_value()) << std::format("Error: {}", result.error());
    }

    void ASSERT_FALSE_RES(const auto& result)
    {
        ASSERT_FALSE(result.has_value()) << std::format("Error: {}", result.error());
    }
} // namespace centipede::test
