#include "centipede/centipede.hpp"
#include "centipede/core/engines/eigen_engine.hpp"
#include <Eigen/Core>
#include <cstddef>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ranges>
#include <vector>

namespace centipede::test
{
    TEST(eigen_engine, constructor)
    {
        constexpr auto n_global_pars = 10;
        auto engine = core::engine::Engine<core::engine::MatrixEngine::eigen, float>{ n_global_pars };
        const auto& factor_matrix = engine.get_global_factor_matrix();
        EXPECT_EQ(factor_matrix.rows(), n_global_pars);
        EXPECT_EQ(factor_matrix.cols(), n_global_pars);
        const auto& rhs_vec = engine.get_global_rhs_vector();
        EXPECT_EQ(rhs_vec.rows(), n_global_pars);
        EXPECT_EQ(rhs_vec.cols(), 1);
    }

    TEST(eigen_engine, solve_rank_deficit)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals = EngineClass::Globals{};
            globals.factor_matrix.resize(3, 3);
            globals.factor_matrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
            globals.factor_matrix = globals.factor_matrix.selfadjointView<Eigen::Upper>();
            globals.rhs_vec.resize(3);
            globals.rhs_vec << 1, 2, 3;
            return globals;
        }();
        EngineClass::solve(globals, result);
        EXPECT_NE(result.error_status, ErrorCode::success);

        EXPECT_EQ(result.rank_deficit, 1);

        const auto redundant_indicies = std::vector<std::size_t>{ 0, 2 };
        EXPECT_EQ(result.redundant_parameter_indices, redundant_indicies);
    }

    TEST(eigen_engine, solve_negative_definite)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals = EngineClass::Globals{};
            globals.factor_matrix.resize(3, 3);
            globals.factor_matrix << 1, 2, 11, 4, 5, 6, 7, 8, 9;
            globals.factor_matrix = globals.factor_matrix.selfadjointView<Eigen::Upper>();
            globals.rhs_vec.resize(3);
            globals.rhs_vec << 1, 2, 3;
            return globals;
        }();
        auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result);
        EXPECT_EQ(result.error_status, ErrorCode::analysis_global_negative_definite)
            << std::format("Error: {}. \n result: {}", result.error_status, result);
    }

    TEST(eigen_engine, solve_zero_factor_matrix)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals = EngineClass::Globals{};
            globals.factor_matrix.resize(3, 3);
            globals.factor_matrix.setZero();
            globals.factor_matrix = globals.factor_matrix.selfadjointView<Eigen::Upper>();
            globals.rhs_vec.resize(3);
            globals.rhs_vec << 1, 2, 3;
            return globals;
        }();
        auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result);
        EXPECT_EQ(result.error_status, ErrorCode::analysis_factor_matrix_zero)
            << std::format("Error: {}.", result.error_status);
    }

    TEST(eigen_engine, solve_zero_rhs_vector)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals = EngineClass::Globals{};
            globals.factor_matrix.resize(3, 3);
            globals.factor_matrix << 11, 2, 3, 4, 5, 6, 7, 8, 9;
            globals.factor_matrix = globals.factor_matrix.selfadjointView<Eigen::Upper>();
            globals.rhs_vec.resize(3);
            globals.rhs_vec.setZero();
            return globals;
        }();
        auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result);
        EXPECT_EQ(result.error_status, ErrorCode::analysis_rhs_vector_zero)
            << std::format("Error: {}.", result.error_status);
    }

    TEST(eigen_engine, solve)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals = EngineClass::Globals{};
            globals.factor_matrix.resize(3, 3);
            globals.factor_matrix << 11, 2, 3, 4, 5, 6, 7, 8, 9;
            globals.factor_matrix = globals.factor_matrix.selfadjointView<Eigen::Upper>();
            globals.rhs_vec.resize(3);
            globals.rhs_vec << 1, 2, 3;
            return globals;
        }();
        auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result);
        EXPECT_EQ(result.error_status, ErrorCode::success)
            << std::format("Error: {}. \n result: {}", result.error_status, result);

        const auto& parameters = result.parameters;

        for (const auto [parameter, val] : std::views::zip(parameters, solution))
        {
            EXPECT_NEAR(parameter.second, val, 1e-4);
        }
    }
} // namespace centipede::test
