#include "centipede/centipede.hpp"
#include "centipede/cli/cxxopts_formatter.hpp"
#include <Eigen/Core>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <sstream>

namespace centipede::testing
{

    TEST(format, error_code)
    {
        constexpr auto enums = magic_enum::enum_values<centipede::ErrorType>();

        for (const auto entry : enums)
        {
            auto error_str = std::format("{}", entry);
            EXPECT_FALSE(error_str.empty());
            if (entry != centipede::ErrorType::invalid)
            {
                EXPECT_NE(error_str, "Error due to no evaluation!");
            }
        }
    }

    TEST(format, error_code_invalid)
    {
        auto err = centipede::ErrorType::invalid;
        auto error_str = std::format("{}", err);
        EXPECT_EQ(error_str, "Error due to no evaluation!");
    }

    TEST(format, result_success)
    {
        auto result = Result<float>{};
        result.error_status = ErrorType::success;
        const auto format_str = std::format("{}", result);
        EXPECT_FALSE(format_str.empty());
    }

    TEST(format, result_error)
    {
        auto result = Result<float>{};
        result.error_status = ErrorType::analysis_local_fit_rank_deficit;
        const auto format_str = std::format("{}", result);
        EXPECT_THAT(format_str, ::testing::HasSubstr(std::format("{}", ErrorType::analysis_local_fit_rank_deficit)));
    }

    TEST(format, eigen_matrices)
    {
        auto mat = Eigen::Matrix3d{}.eval();
        const auto format_str = std::format("{}", mat);
        EXPECT_FALSE(format_str.empty());
    }

    TEST(format, spdlog_stream)
    {
        auto log = spdlog::level::level_enum{};
        std::istringstream{ "err" } >> log;
        EXPECT_EQ(log, spdlog::level::err);

        auto isstream = std::istringstream{ "invalid" };
        isstream >> log;
        EXPECT_TRUE(isstream.fail());
    }
} // namespace centipede::testing
