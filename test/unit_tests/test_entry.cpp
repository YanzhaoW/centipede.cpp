#include "centipede/centipede.hpp"
#include <array>
#include <format>
#include <gtest/gtest.h>
#include <utility>

TEST(static_entrypoint, constructor) { auto entry = centipede::EntryPoint<1, 1>{}; }

TEST(dynamic_entrypoint, constructor) { auto entry = centipede::EntryPoint{}; }

TEST(static_entrypoint, setters)
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

TEST(dynamic_entrypoint, setters)
{
    // NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers)
    auto entry = centipede::EntryPoint{};
    entry.set_locals(1., 3.).set_globals(std::pair{ 3, 1. }, std::pair{ 10, 2. }, std::pair{ 11, 3. });
    entry.set_locals(1., 3.F).set_globals(std::pair{ 3, 1.F }, std::pair{ 10, 2. }, std::pair{ 11, 3. });
    auto is_equal = entry.get_locals() == centipede::EntryPoint<>::LocalDerivs{ 1.F, 3.F };
    EXPECT_TRUE(is_equal);
    is_equal = entry.get_globals() ==
               centipede::EntryPoint<>::GlobalDerivs{ std::pair{ 3, 1. }, std::pair{ 10, 2. }, std::pair{ 11, 3. } };
    // NOLINTEND (cppcoreguidelines-avoid-magic-numbers)
    EXPECT_TRUE(is_equal);
}

TEST(static_entrypoint, clear)
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

TEST(dynamic_entrypoint, clear)
{
    auto entry =
        centipede::EntryPoint{}.set_locals(1.F).set_globals(std::pair{ 1U, 1.F }).set_measurement(1.).set_sigma(1.);

    entry.reset();

    EXPECT_EQ(entry.get_globals().size(), 0);
    EXPECT_EQ(entry.get_locals().size(), 0);
    EXPECT_EQ(entry.get_measurement(), 0.F);
    EXPECT_EQ(entry.get_sigma(), 0.F);
}

TEST(static_entrypoint, format)
{
    auto entry = centipede::EntryPoint<1, 1>{}
                     .set_locals(1.F)
                     .set_globals(std::pair{ 1U, 1.F })
                     .set_measurement(1.)
                     .set_sigma(1.);

    auto format_str = std::format("{}", entry);
    EXPECT_STREQ(format_str.data(), "local derivatives: [1], global derivatives: [(1, 1)], measurement: 1, sigma: 1");
}

TEST(dynamic_entrypoint, format)
{
    auto entry =
        centipede::EntryPoint{}.set_locals(1.F).set_globals(std::pair{ 1U, 1.F }).set_measurement(1.).set_sigma(1.);

    auto format_str = std::format("{}", entry);
    EXPECT_STREQ(format_str.data(), "local derivatives: [1], global derivatives: [(1, 1)], measurement: 1, sigma: 1");
}

// NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers)
TEST(dynamic_entrypoint, add_elements)
{
    auto entry = centipede::EntryPoint{}.add_local(1.).add_local(2.F).add_local(3.);
    auto local_test = std::vector{ 1.F, 2.F, 3.F };
    EXPECT_EQ(entry.get_locals(), local_test);
    entry.add_global(0, 2.).add_global(2, 2.2).add_global(4, 1.9).add_global(8, 2.8);
    auto global_test = std::vector{
        std::pair{ 0U, 2.F },
        std::pair{ 2U, 2.2F },
        std::pair{ 4U, 1.9F },
        std::pair{ 8U, 2.8F },
    };
    EXPECT_EQ(entry.get_globals(), global_test);
}
// NOLINTEND (cppcoreguidelines-avoid-magic-numbers)
