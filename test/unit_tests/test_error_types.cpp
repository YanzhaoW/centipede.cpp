#include "centipede/centipede.hpp"
#include <format>
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>

TEST(error_type, format)
{
    constexpr auto enums = magic_enum::enum_values<centipede::ErrorCode>();

    for (const auto entry : enums)
    {
        auto error_str = std::format("{}", entry);
        EXPECT_FALSE(error_str.empty());
        if (entry != centipede::ErrorCode::invalid)
        {
            EXPECT_NE(error_str, "invalid error code");
        }
    }
}

TEST(error_type, format_error)
{
    auto err = centipede::ErrorCode::invalid;
    auto error_str = std::format("{}", err);
    EXPECT_EQ(error_str, "invalid error code");
}
