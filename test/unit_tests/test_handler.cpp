#include "centipede/centipede.hpp"
#include "shared.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace centipede::test
{
    TEST(handler, constructor) { auto handler = centipede::create({ .n_globals = DEFAULT_MAX_GLOBAL_ID }); }

    TEST(handler, constructor_float_eigen)
    {
        auto handler = centipede::create<float>({ .n_globals = DEFAULT_MAX_GLOBAL_ID });
        ASSERT_TRUE(handler);
    }

    TEST(handler, constructor_double_eigen)
    {
        auto handler = centipede::create<double>({ .n_globals = DEFAULT_MAX_GLOBAL_ID });
        ASSERT_TRUE(handler);
    }

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    TEST(handler, add_entrypoint)
    {
        auto handler = centipede::create({ .n_globals = DEFAULT_MAX_GLOBAL_ID });
        ASSERT_TRUE(handler);
        constexpr auto n_points = 100;
        const auto entrypoints = generate_random_entry_points(n_points);
        EXPECT_EQ(n_points, entrypoints.size());

        for (const auto& entry_point : entrypoints)
        {
            auto err = handler.value().add_entrypoint(entry_point);
            EXPECT_TRUE(err.has_value());
        }
        const auto& state = handler.value().get_current_state();
        const auto& current_entry = state.entry;
        EXPECT_EQ(current_entry.n_locals, DEFAULT_N_LOCALS);
        EXPECT_EQ(current_entry.local_derivs.size(), DEFAULT_N_LOCALS * n_points);
        EXPECT_EQ(current_entry.global_derivs.size(), DEFAULT_N_GLOBALS * n_points);
        EXPECT_EQ(current_entry.measurements.size(), n_points);
        EXPECT_EQ(current_entry.sigmas.size(), n_points);
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    TEST(handler, empty_entry)
    {
        auto handler = centipede::create<double>({ .n_globals = DEFAULT_MAX_GLOBAL_ID });

        ASSERT_TRUE(handler);
        auto res = handler.value().analyze_current_entry();
        EXPECT_FALSE(res);
        EXPECT_EQ(res.error(), ErrorCode::analysis_empty_entry);
    }

    TEST(handler, local_derivs_incomp_numbers)
    {
        auto handler = centipede::create<double>({ .n_globals = DEFAULT_MAX_GLOBAL_ID });

        ASSERT_TRUE(handler);
        constexpr auto n_points = 10;
        const auto entrypoints = generate_random_entry_points(n_points);
        for (const auto& entry_point : entrypoints)
        {
            auto err = handler.value().add_entrypoint(entry_point);
            EXPECT_TRUE(err.has_value());
        }
        const auto new_entrypoints = generate_random_entry_points(1, 1, 1);
        for (const auto& entry_point : new_entrypoints)
        {
            auto err = handler.value().add_entrypoint(entry_point);
            ASSERT_FALSE(err.has_value());
            EXPECT_EQ(err.error(), centipede::ErrorCode::handler_incomp_n_locals);
        }
    }

    TEST(handler, n_globals_too_small)
    {
        auto handler = centipede::create<double>({ .n_globals = 3, .fixed_parameter_ids = { 1, 2, 3 } });

        ASSERT_FALSE(handler);
    }
} // namespace centipede::test
