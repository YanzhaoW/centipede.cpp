#include "centipede/centipede.hpp"
#include "shared.hpp"
#include <cstddef>
#include <expected>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <utility>

namespace
{
    namespace engine = centipede::core::engine;
} // namespace

namespace centipede::core::engine
{
    namespace
    {
        struct GlobalsType
        {
        };

        template <typename DataType>
        class MockHelper
        {
          public:
            MOCK_METHOD(void, construct_with, (std::size_t n_globals), ());
            MOCK_METHOD(void, solve, (const GlobalsType& globals, Result<DataType>& result), ());
        };
    } // namespace

    /** @cond */

    template <typename DataType>
    class Engine<MatrixEngine::mock, DataType>
    {
      public:
        Engine(std::size_t n_globals) { mock_helper->construct_with(n_globals); }
        using Globals = GlobalsType;

        static void solve(const Globals& globals, Result<DataType>& result) { mock_helper->solve(globals, result); }

        MOCK_METHOD(void, add_to_globals, (Globals & globals), (const));
        MOCK_METHOD((VoidError), fill_data, (const Entry<DataType>& entry), (const));
        MOCK_METHOD((void), add_to_result, (Result<DataType> & result), (const));
        MOCK_METHOD((EnumError<>), analyze, (double alpha), (const));

        static MockHelper<DataType>* mock_helper;
    };

    template <typename DataType>
    MockHelper<DataType>* Engine<MatrixEngine::mock, DataType>::mock_helper = nullptr;
    /** @endcond */
} // namespace centipede::core::engine

namespace centipede::test
{
    namespace
    {
        class master_engine : public testing::Test
        {
          public:
            using Master = engine::Master<float, { .engine_type = engine::MatrixEngine::mock }>;
            using Config = Master::Config;

          protected:
            using MockHelperType = core::engine::MockHelper<Master::DataTypeUsed>;
            using EngineClass = Master::EngineImp;
            using ResultType = Result<Master::DataTypeUsed>;

            void SetUp() override
            {
                mock_helper_ = std::make_unique<MockHelperType>();
                Master::EngineImp::mock_helper = mock_helper_.get();
                EXPECT_CALL(*mock_helper_, construct_with(DEFAULT_MAX_GLOBAL_ID)).Times(1);
                master_ = std::make_unique<Master>(Config{ .n_globals = DEFAULT_MAX_GLOBAL_ID });
                engine_class_ = &(master_->get_engine());
            }

            void TearDown() override { Master::EngineImp::mock_helper = nullptr; }

            std::unique_ptr<MockHelperType> mock_helper_;
            std::unique_ptr<Master> master_;
            const EngineClass* engine_class_;
        };
    } // namespace
    TEST_F(master_engine, constructor) {}

    TEST_F(master_engine, add_entrypoint_dynamic)
    {
        const auto entrypoint = EntryPoint<>{}
                                    .set_measurement(3.F)
                                    .set_sigma(2.F)
                                    .set_globals(std::pair{ 9, 2.F }, std::pair{ 2, 3.F })
                                    .set_locals(1.5F, 2.5F);
        const auto& state = master_->get_current_state();
        EXPECT_EQ(state.next_point_index, 0);
        EXPECT_FALSE(state.entry.n_locals);
        EXPECT_EQ(state.entry.measurements.size(), 0);
        EXPECT_EQ(state.entry.sigmas.size(), 0);
        EXPECT_EQ(state.entry.local_derivs.size(), 0);
        EXPECT_EQ(state.entry.global_derivs.size(), 0);

        EXPECT_TRUE_RES(master_->add_entrypoint(entrypoint));

        EXPECT_EQ(state.next_point_index, 1);
        ASSERT_TRUE(state.entry.n_locals);
        EXPECT_EQ(state.entry.n_locals.value(), 2);
        EXPECT_EQ(state.entry.measurements.size(), 1);
        EXPECT_EQ(state.entry.sigmas.size(), 1);
        EXPECT_EQ(state.entry.local_derivs.size(), 2);

        EXPECT_EQ(state.entry.local_derivs.at(0).first, 0);
        EXPECT_EQ(state.entry.local_derivs.at(0).second.first, 0);
        EXPECT_EQ(state.entry.local_derivs.at(0).second.second, 1.5F);

        EXPECT_EQ(state.entry.local_derivs.at(1).first, 0);
        EXPECT_EQ(state.entry.local_derivs.at(1).second.first, 1);
        EXPECT_EQ(state.entry.local_derivs.at(1).second.second, 2.5F);

        EXPECT_EQ(state.entry.global_derivs.size(), 2);

        EXPECT_EQ(state.entry.global_derivs.at(0).first, 0);
        EXPECT_EQ(state.entry.global_derivs.at(0).second.first, 2);
        EXPECT_EQ(state.entry.global_derivs.at(0).second.second, 3.F);

        EXPECT_EQ(state.entry.global_derivs.at(1).first, 0);
        EXPECT_EQ(state.entry.global_derivs.at(1).second.first, 9);
        EXPECT_EQ(state.entry.global_derivs.at(1).second.second, 2.F);
    }

    TEST_F(master_engine, add_entrypoint_incomp_n_locals)
    {
        const auto first_entrypoint = EntryPoint<>{}
                                          .set_measurement(3.F)
                                          .set_sigma(2.F)
                                          .set_globals(std::pair{ 9, 2.F }, std::pair{ 2, 3.F })
                                          .set_locals(1.5F, 2.5F);
        const auto second_entrypoint = EntryPoint<>{}
                                           .set_measurement(3.F)
                                           .set_sigma(2.F)
                                           .set_globals(std::pair{ 9, 2.F }, std::pair{ 2, 3.F })
                                           .set_locals(1.5F, 2.5F, 3.4F);
        const auto& state = master_->get_current_state();

        EXPECT_TRUE_RES(master_->add_entrypoint(first_entrypoint));
        auto res = master_->add_entrypoint(second_entrypoint);
        ASSERT_FALSE(res);
        EXPECT_EQ(res.error(), ErrorCode::handler_incomp_n_locals);
    }

    TEST_F(master_engine, analyze)
    {
        const auto& engine = master_->get_engine();
        EXPECT_CALL(engine, analyze(testing::_)).WillOnce(testing::Return(EnumError<>{}));
        EXPECT_CALL(engine, fill_data(testing::_));

        EXPECT_TRUE_RES(master_->analyze());

        const auto& state = master_->get_current_state();
        EXPECT_EQ(state.next_point_index, 0);
        EXPECT_FALSE(state.entry.n_locals);
        EXPECT_EQ(state.entry.measurements.size(), 0);
        EXPECT_EQ(state.entry.sigmas.size(), 0);
        EXPECT_EQ(state.entry.local_derivs.size(), 0);
        EXPECT_EQ(state.entry.global_derivs.size(), 0);
    }

    TEST_F(master_engine, analyze_fail)
    {
        const auto& engine = master_->get_engine();
        EXPECT_CALL(engine, analyze(testing::_))
            .WillOnce(testing::Return(std::unexpected{ ErrorCode::analysis_rank_deficit }));
        EXPECT_CALL(engine, fill_data(testing::_));

        auto res = master_->analyze();
        EXPECT_FALSE(res);
        EXPECT_EQ(res.error(), ErrorCode::analysis_rank_deficit);
    }

    TEST_F(master_engine, solve)
    {
        EXPECT_CALL(*engine_class_, add_to_globals(testing::_)).Times(1);
        EXPECT_CALL(*engine_class_, add_to_result(testing::_)).Times(1);
        EXPECT_CALL(*mock_helper_, solve(testing::_, testing::_))
            .Times(1)
            .WillOnce([](const auto&, ResultType& result) { result.error_status = ErrorCode::success; });

        EXPECT_TRUE_RES(master_->solve());
    }

    TEST_F(master_engine, solve_fail)
    {
        EXPECT_CALL(*engine_class_, add_to_globals(testing::_)).Times(1);
        EXPECT_CALL(*engine_class_, add_to_result(testing::_)).Times(1);
        EXPECT_CALL(*mock_helper_, solve(testing::_, testing::_))
            .Times(1)
            .WillOnce([](const auto&, ResultType& result) { result.error_status = ErrorCode::analysis_rank_deficit; });

        auto res = master_->solve();
        EXPECT_FALSE(res);
        EXPECT_EQ(res.error(), ErrorCode::analysis_rank_deficit);
    }
} // namespace centipede::test
