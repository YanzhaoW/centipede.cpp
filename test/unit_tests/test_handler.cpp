#include "centipede/centipede.hpp"
#include "shared.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace
{
    constexpr auto DEFAULT_MAX_POINT = 30;
    using centipede::Handler;

} // namespace

namespace centipede::test
{
    TEST(handler, constructor) { auto handler = Handler{}; }

    TEST(handler, constructor_float_eigen)
    {
        using HandlerType = Handler<float>;
        using Config = HandlerType::Config;
        auto handler = HandlerType{ Config{ .n_globals = DEFAULT_MAX_GLOBAL_ID } };
    }

    TEST(handler, constructor_double_eigen)
    {
        using HandlerType = Handler<double>;
        using Config = HandlerType::Config;
        auto handler = HandlerType{ Config{ .n_globals = DEFAULT_MAX_GLOBAL_ID } };
    }

    TEST(handler, init)
    {
        auto handler = Handler{};
        auto err = handler.init();
        EXPECT_TRUE(err.has_value());
    }

    // NOLINTBEGIN(readability-function-cognitive-complexity)
    TEST(handler, add_entrypoint)
    {
        auto handler = Handler{};
        auto err = handler.init();
        EXPECT_TRUE(err.has_value());
        constexpr auto n_points = 100;
        const auto entrypoints = generate_random_entry_points(n_points);
        EXPECT_EQ(n_points, entrypoints.size());

        for (const auto& entry_point : entrypoints)
        {
            auto err = handler.add_entrypoint(entry_point);
            EXPECT_TRUE(err.has_value());
        }
        const auto& state = handler.get_current_state();
        const auto& current_entry = state.entry;
        EXPECT_EQ(current_entry.n_locals, DEFAULT_N_LOCALS);
        EXPECT_EQ(current_entry.local_derivs.size(), DEFAULT_N_LOCALS * n_points);
        EXPECT_EQ(current_entry.global_derivs.size(), DEFAULT_N_GLOBALS * n_points);
        EXPECT_EQ(current_entry.measurements.size(), n_points);
        EXPECT_EQ(current_entry.sigmas.size(), n_points);
    }
    // NOLINTEND(readability-function-cognitive-complexity)

    TEST(handler, local_derivs_incomp_numbers)
    {
        auto handler = Handler{};
        auto err = handler.init();
        EXPECT_TRUE(err.has_value());
        constexpr auto n_points = 10;
        const auto entrypoints = generate_random_entry_points(n_points);
        for (const auto& entry_point : entrypoints)
        {
            auto err = handler.add_entrypoint(entry_point);
            EXPECT_TRUE(err.has_value());
        }
        const auto new_entrypoints = generate_random_entry_points(1, 1, 1);
        for (const auto& entry_point : new_entrypoints)
        {
            auto err = handler.add_entrypoint(entry_point);
            ASSERT_FALSE(err.has_value());
            EXPECT_EQ(err.error(), centipede::ErrorCode::handler_incomp_n_locals);
        }
    }
} // namespace centipede::test
