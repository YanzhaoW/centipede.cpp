#include "centipede/centipede.hpp"
#include "centipede/core/engines/eigen_engine.hpp"
#include "centipede/core/engines/par_id_map.hpp"
#include <Eigen/Core>
#include <cstddef>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

namespace centipede::test
{
    // NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers)
    TEST(eigen_engine, constructor)
    {
        constexpr auto n_global_pars = 10;
        const auto config = Config<float>{ .n_globals = n_global_pars };
        auto engine = core::engine::Engine<core::engine::MatrixEngine::eigen, float>{ config };
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
            auto globals_tmp = EngineClass::Globals{};
            globals_tmp.factor_matrix.resize(3, 3);
            globals_tmp.factor_matrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;
            globals_tmp.factor_matrix = globals_tmp.factor_matrix.selfadjointView<Eigen::Upper>();
            globals_tmp.rhs_vec.resize(3);
            globals_tmp.rhs_vec << 1, 2, 3;
            return globals_tmp;
        }();
        EngineClass::solve(globals, result, core::ParIdMap{ 3 });
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
            auto globals_tmp = EngineClass::Globals{};
            globals_tmp.factor_matrix.resize(3, 3);
            globals_tmp.factor_matrix << 1, 2, 11, 4, 5, 6, 7, 8, 9;
            globals_tmp.factor_matrix = globals_tmp.factor_matrix.selfadjointView<Eigen::Upper>();
            globals_tmp.rhs_vec.resize(3);
            globals_tmp.rhs_vec << 1, 2, 3;
            return globals_tmp;
        }();
        [[maybe_unused]] auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result, core::ParIdMap{ 3 });
        EXPECT_EQ(result.error_status, ErrorCode::analysis_global_negative_definite)
            << std::format("Error: {}. \n result: {}", result.error_status, result);
    }

    TEST(eigen_engine, solve_zero_factor_matrix)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals_tmp = EngineClass::Globals{};
            globals_tmp.factor_matrix.resize(3, 3);
            globals_tmp.factor_matrix.setZero();
            globals_tmp.factor_matrix = globals_tmp.factor_matrix.selfadjointView<Eigen::Upper>();
            globals_tmp.rhs_vec.resize(3);
            globals_tmp.rhs_vec << 1, 2, 3;
            return globals_tmp;
        }();
        [[maybe_unused]] auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result, core::ParIdMap{ 3 });
        EXPECT_EQ(result.error_status, ErrorCode::analysis_factor_matrix_zero)
            << std::format("Error: {}.", result.error_status);
    }

    TEST(eigen_engine, solve_zero_rhs_vector)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals_tmp = EngineClass::Globals{};
            globals_tmp.factor_matrix.resize(3, 3);
            globals_tmp.factor_matrix << 11, 2, 3, 4, 5, 6, 7, 8, 9;
            globals_tmp.factor_matrix = globals_tmp.factor_matrix.selfadjointView<Eigen::Upper>();
            globals_tmp.rhs_vec.resize(3);
            globals_tmp.rhs_vec.setZero();
            return globals_tmp;
        }();
        [[maybe_unused]] auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result, core::ParIdMap{ 3 });
        EXPECT_EQ(result.error_status, ErrorCode::analysis_rhs_vector_zero)
            << std::format("Error: {}.", result.error_status);
    }

    TEST(eigen_engine, solve)
    {
        using EngineClass = core::engine::Engine<core::engine::MatrixEngine::eigen, float>;

        auto result = Result<float>{};
        const auto globals = []()
        {
            auto globals_tmp = EngineClass::Globals{};
            globals_tmp.factor_matrix.resize(3, 3);
            globals_tmp.factor_matrix << 11, 2, 3, 4, 5, 6, 7, 8, 9;
            globals_tmp.factor_matrix = globals_tmp.factor_matrix.selfadjointView<Eigen::Upper>();
            globals_tmp.rhs_vec.resize(3);
            globals_tmp.rhs_vec << 1, 2, 3;
            return globals_tmp;
        }();
        auto solution = globals.factor_matrix.inverse() * globals.rhs_vec;
        EngineClass::solve(globals, result, core::ParIdMap{ 3 });
        EXPECT_EQ(result.error_status, ErrorCode::success)
            << std::format("Error: {}. \n result: {}", result.error_status, result);

        const auto& parameters = result.parameters;

        for (const auto& [idx, val] : parameters)
        {
            EXPECT_NEAR(val, solution[static_cast<Eigen::Index>(idx)], 1e-4)
                << std::format("parameters: {}\n solutions: {}", parameters, solution);
        }
    }
    // NOLINTEND (cppcoreguidelines-avoid-magic-numbers)
} // namespace centipede::test
