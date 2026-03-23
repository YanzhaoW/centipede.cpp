#include "centipede/centipede.hpp"
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>

namespace centipede::testing
{

    TEST(format, error_code)
    {
        constexpr auto enums = magic_enum::enum_values<centipede::ErrorCode>();

        for (const auto entry : enums)
        {
            auto error_str = std::format("{}", entry);
            EXPECT_FALSE(error_str.empty());
            if (entry != centipede::ErrorCode::invalid)
            {
                EXPECT_NE(error_str, "Error due to no evaluation!");
            }
        }
    }

    TEST(format, error_code_invalid)
    {
        auto err = centipede::ErrorCode::invalid;
        auto error_str = std::format("{}", err);
        EXPECT_EQ(error_str, "Error due to no evaluation!");
    }

    TEST(format, result_success)
    {
        auto result = Result<float>{};
        result.error_status = ErrorCode::success;
        const auto format_str = std::format("{}", result);
        EXPECT_FALSE(format_str.empty());
    }

    TEST(format, result_error)
    {
        auto result = Result<float>{};
        result.error_status = ErrorCode::analysis_local_fit_rank_deficit;
        const auto format_str = std::format("{}", result);
        EXPECT_THAT(format_str, ::testing::HasSubstr(std::format("{}", ErrorCode::analysis_local_fit_rank_deficit)));
    }
} // namespace centipede::testing
