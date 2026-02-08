#include "centipede/centipede.hpp"
#include <array>
#include <format>
#include <gtest/gtest.h>
#include <utility>

TEST(entrypoint, constructor) { auto entry = centipede::EntryPoint<1, 1>{}; }

TEST(entrypoint, setters)
{
    // NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers)
    auto entry = centipede::EntryPoint<2, 3>{};
    entry.set_locals(1., 3.).set_globals(std::pair{ 3, 1. }, std::pair{ 10, 2. }, std::pair{ 11, 3. });
    entry.set_locals(1., 3.F).set_globals(std::pair{ 3, 1.F }, std::pair{ 10, 2. }, std::pair{ 11, 3. });
    auto is_equal = entry.get_locals() == centipede::EntryPoint<2, 3>::LocalDerivs{ 1.F, 3.F };
    EXPECT_TRUE(is_equal);
    is_equal =
        entry.get_globals() ==
        centipede::EntryPoint<2, 3>::GlobalDerivs{ std::pair{ 3, 1. }, std::pair{ 10, 2. }, std::pair{ 11, 3. } };
    // NOLINTEND (cppcoreguidelines-avoid-magic-numbers)
    EXPECT_TRUE(is_equal);
}

TEST(entrypoint, clear)
{
    auto entry = centipede::EntryPoint<1, 1>{}
                     .set_locals(1.F)
                     .set_globals(std::pair{ 1U, 1.F })
                     .set_measurement(1.)
                     .set_sigma(1.);
    auto old_entry = entry;

    entry.reset();

    auto default_entry = decltype(entry){};
    EXPECT_EQ(entry, default_entry);
    EXPECT_NE(entry, old_entry);
}

TEST(entrypoint, format)
{
    auto entry = centipede::EntryPoint<1, 1>{}
                     .set_locals(1.F)
                     .set_globals(std::pair{ 1U, 1.F })
                     .set_measurement(1.)
                     .set_sigma(1.);

    auto format_str = std::format("{}", entry);
    EXPECT_STREQ(format_str.data(), "local derivatives: [1], global derivatives: [(1, 1)], measurement: 1, sigma: 1");
}
