#include "centipede/centipede.hpp"
#include <array>
#include <format>
#include <gtest/gtest.h>
#include <utility>

TEST(entrypoint, constructor) { auto entry = centipede::EntryPoint<1, 1>{}; }

TEST(entrypoint, clear)
{
    auto entry = centipede::EntryPoint{ .local_derivs = std::array{ 1.F },
                                        .global_derivs = std::array{ std::pair{ 1U, 1.F } },
                                        .measurement = 1.F,
                                        .sigma = 1.F };
    auto old_entry = entry;

    entry.reset();

    auto default_entry = decltype(entry){};
    EXPECT_EQ(entry, default_entry);
    EXPECT_NE(entry, old_entry);
}

TEST(entrypoint, format)
{
    auto entry = centipede::EntryPoint{ .local_derivs = std::array{ 1.F },
                                        .global_derivs = std::array{ std::pair{ 1U, 1.F } },
                                        .measurement = 1.F,
                                        .sigma = 1.F };

    auto format_str = std::format("{}", entry);
    EXPECT_STREQ(format_str.data(), "local derivatives: [1], global derivatives: [(1, 1)], measurement: 1, sigma: 1");
}
