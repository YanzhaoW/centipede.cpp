#include "centipede/centipede.hpp"
#include "shared.hpp"
#include <cstddef>
#include <expected>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

namespace centipede::test
{
    namespace
    {
        template <typename DataType>
        class DerivedBaseEngine : public core::engine::Base<DataType>
        {
          public:
            using Base = core::engine::Base<DataType>;

            DerivedBaseEngine(std::size_t n_globals)
                : Base{ n_globals }
            {
            }

            // Called in fill_data method.
            MOCK_METHOD(void, resize_buffers, (), ());
            MOCK_METHOD(void, fill_measurements, (const std::vector<DataType>&), ());
            MOCK_METHOD(void, fill_sigmas, (const std::vector<DataType>&), ());
            MOCK_METHOD(void, fill_local_derivs, (const std::vector<typename Entry<DataType>::Deriv>&), ());
            MOCK_METHOD(void, fill_global_derivs, (const std::vector<typename Entry<DataType>::Deriv>&), ());

            // Called in analyze method.
            MOCK_METHOD((EnumError<>), fit_local_pars, (), ());
            MOCK_METHOD((std::pair<std::size_t, double>), calculate_local_fit_chi_square, (), ());
            MOCK_METHOD((void), update_global_factor_matrix, (), ());
            MOCK_METHOD((void), update_global_rhs_vector, (), ());
        };
    } // namespace

    TEST(base_engine, constructor)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        const auto& state = engine.get_current_state();

        EXPECT_EQ(state.n_globals, 10z);
        EXPECT_FALSE(state.is_rejected);
        EXPECT_EQ(state.n_locals, 0z);
        EXPECT_EQ(state.n_points, 0z);
        EXPECT_EQ(state.ndf, 0z);
        EXPECT_EQ(state.chi2, 0.);
        EXPECT_EQ(state.p_value, 0.);
    }

    TEST(base_engine, fill_data)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        const auto& state = engine.get_current_state();

        const auto entry = []()
        {
            auto entry = Entry<float>{};
            using Deriv = Entry<float>::Deriv;
            entry.n_locals = 3;
            entry.measurements = std::vector{ 11.F, 12.F, 13.F };
            entry.sigmas = std::vector{ 1.F, 2.F, 3.F };
            entry.local_derivs = std::vector{ Deriv{ 0, std::pair{ 0, 1.F } }, Deriv{ 0, std::pair{ 1, 1.F } },
                                              Deriv{ 1, std::pair{ 0, 1.F } }, Deriv{ 1, std::pair{ 1, 1.F } },
                                              Deriv{ 2, std::pair{ 0, 1.F } }, Deriv{ 2, std::pair{ 1, 1.F } } };
            entry.global_derivs = std::vector{ Deriv{ 0, std::pair{ 0, 1.F } }, Deriv{ 0, std::pair{ 1, 1.F } },
                                               Deriv{ 1, std::pair{ 2, 1.F } }, Deriv{ 1, std::pair{ 3, 1.F } },
                                               Deriv{ 2, std::pair{ 7, 1.F } }, Deriv{ 2, std::pair{ 4, 1.F } } };
            return entry;
        }();

        EXPECT_CALL(engine, resize_buffers()).Times(1);
        EXPECT_CALL(engine, fill_measurements(entry.measurements)).Times(1);
        EXPECT_CALL(engine, fill_sigmas(entry.sigmas)).Times(1);
        EXPECT_CALL(engine, fill_local_derivs(entry.local_derivs)).Times(1);
        EXPECT_CALL(engine, fill_global_derivs(entry.global_derivs)).Times(1);

        engine.fill_data(entry);

        EXPECT_EQ(engine.get_log().n_entries_read, 1);
    }

    TEST(base_engine, fill_empty_data)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };
        const auto entry = Entry<float>{};
        EXPECT_CALL(engine, resize_buffers()).Times(0);
        EXPECT_CALL(engine, fill_measurements(entry.measurements)).Times(0);
        EXPECT_CALL(engine, fill_sigmas(entry.sigmas)).Times(0);
        EXPECT_CALL(engine, fill_local_derivs(entry.local_derivs)).Times(0);
        EXPECT_CALL(engine, fill_global_derivs(entry.global_derivs)).Times(0);

        engine.fill_data(entry);
        EXPECT_EQ(engine.get_log().n_entries_read, 0);
    }

    TEST(base_engine, analyze)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        EXPECT_CALL(engine, fit_local_pars()).Times(1).WillOnce(testing::Return(EnumError<>{}));
        EXPECT_CALL(engine, calculate_local_fit_chi_square())
            .Times(1)
            .WillOnce(testing::Return(std::pair<std::size_t, double>{ 1.F, 0.999 }));
        EXPECT_CALL(engine, update_global_factor_matrix()).Times(1);
        EXPECT_CALL(engine, update_global_rhs_vector()).Times(1);

        const auto alpha = 0.0001;
        EXPECT_TRUE_RES(engine.analyze(alpha));
        EXPECT_EQ(engine.get_log().n_entries_success, 1);
    }

    TEST(base_engine, analyze_local_rank_deficit)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        EXPECT_CALL(engine, fit_local_pars())
            .Times(1)
            .WillOnce(testing::Return(std::unexpected{ ErrorCode::analysis_local_fit_rank_deficit }));
        EXPECT_CALL(engine, calculate_local_fit_chi_square()).Times(0);
        EXPECT_CALL(engine, update_global_factor_matrix()).Times(0);
        EXPECT_CALL(engine, update_global_rhs_vector()).Times(0);

        const auto alpha = 0.0001;
        auto res = engine.analyze(alpha);
        ASSERT_FALSE(res);
        EXPECT_EQ(engine.get_log().n_entries_local_rank_deficit, 1);
        EXPECT_EQ(res.error(), ErrorCode::analysis_local_fit_rank_deficit);
    }

    TEST(base_engine, analyze_low_state)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        EXPECT_CALL(engine, fit_local_pars())
            .Times(1)
            .WillOnce(testing::Return(std::unexpected{ ErrorCode::analysis_local_fit_low_stat }));
        EXPECT_CALL(engine, calculate_local_fit_chi_square()).Times(0);
        EXPECT_CALL(engine, update_global_factor_matrix()).Times(0);
        EXPECT_CALL(engine, update_global_rhs_vector()).Times(0);

        const auto alpha = 0.0001;
        auto res = engine.analyze(alpha);
        ASSERT_FALSE(res);
        EXPECT_EQ(engine.get_log().n_entries_low_stat, 1);
        EXPECT_EQ(res.error(), ErrorCode::analysis_local_fit_low_stat);
    }

    TEST(base_engine, analyze_reject)
    {
        auto engine = DerivedBaseEngine<float>{ 10z };

        EXPECT_CALL(engine, fit_local_pars()).Times(1).WillOnce(testing::Return(EnumError<>{}));
        EXPECT_CALL(engine, calculate_local_fit_chi_square())
            .Times(1)
            .WillOnce(testing::Return(std::pair<std::size_t, double>{ 10, 1000000 }));
        EXPECT_CALL(engine, update_global_factor_matrix()).Times(0);
        EXPECT_CALL(engine, update_global_rhs_vector()).Times(0);

        const auto alpha = 0.0001;
        auto err = engine.analyze(alpha);
        ASSERT_FALSE(err);
        EXPECT_EQ(err.error(), ErrorCode::analysis_local_fit_rejected);
        EXPECT_EQ(engine.get_log().n_entries_rejected, 1);
    }

} // namespace centipede::test
