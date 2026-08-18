#include "centipede/centipede.hpp"
#include <cstdint>
#include <filesystem>
#include <format>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

using centipede::ErrorCode;
using centipede::writer::Binary;
namespace fs = std::filesystem;

// NOLINTBEGIN (cppcoreguidelines-avoid-magic-numbers)
TEST(writer, constructor)
{
    auto writer = Binary{ { .out_filename = "binary_writer_constructor.bin" } };
    EXPECT_FALSE(fs::exists(fs::path(writer.get_config().out_filename)));
}

TEST(writer, init)
{
    auto writer = Binary{ { .out_filename = "binary_writer_init.bin" } };
    auto error = writer.init();

    EXPECT_TRUE(error.has_value());
    EXPECT_TRUE(fs::exists(fs::path(writer.get_config().out_filename)));

    const auto& buffer = writer.get_buffer();
    EXPECT_EQ(buffer.first, std::vector{ 0U });
    EXPECT_EQ(buffer.second, std::vector{ 0.F });
}

TEST(writer, init_error)
{
    auto writer = Binary{ { .out_filename = "" } };
    auto error = writer.init();

    EXPECT_TRUE(not error.has_value());
    EXPECT_EQ(error.error(), ErrorCode::writer_file_fail_to_open);
    EXPECT_FALSE(fs::exists(fs::path(writer.get_config().out_filename)));
}

namespace
{
    const auto valid_meas = 1.F;
    const auto valid_sigma = 1.F;
    const auto valid_entry_point = centipede::EntryPoint<3, 2>{}
                                       .set_locals(1.F, 2.F, 3.F)
                                       .set_globals(std::pair{ 10U, 2.F }, std::pair{ 11U, 3.F })
                                       .set_measurement(valid_meas)
                                       .set_sigma(valid_sigma);
} // namespace
TEST(writer, read_entrypoint_normal)
{
    auto writer = Binary{ {} };
    [[maybe_unused]] auto init_err = writer.init();

    auto err = writer.add_entrypoint(valid_entry_point);
    ASSERT_TRUE(err.has_value());

    const auto& buffer = writer.get_buffer();
    const auto idx_vec = std::vector<uint32_t>{ 0U, 0U, 1U, 2U, 3U, 0U, 11U, 12U };
    const auto val_vec = std::vector<float>{ 0.F, valid_meas, 1.F, 2.F, 3.F, valid_sigma, 2.F, 3.F };
    EXPECT_EQ(buffer.first, idx_vec);
    EXPECT_EQ(buffer.second, val_vec);
}

TEST(writer, read_entrypoint_local_zero)
{
    auto writer = Binary{ {} };
    [[maybe_unused]] auto init_err = writer.init();

    auto entry_point = centipede::EntryPoint<>{};
    entry_point.set_locals(0.F, 1.0F)
        .set_globals(std::pair{ 10U, 1.F }, std::pair{ 11U, 2.F })
        .set_measurement(2.3F)
        .set_sigma(1.2F);
    auto is_ok = writer.add_entrypoint(entry_point);
    ASSERT_TRUE(is_ok);
    const auto buffer = writer.get_buffer();
    EXPECT_EQ(buffer.second.size(), buffer.first.size());
    EXPECT_EQ(buffer.first.size(), 7) << std::format("buffer: {}\n entrypoint: {}", buffer, entry_point);
}

TEST(writer, read_entrypoint_reject)
{
    auto writer = Binary{ {} };
    [[maybe_unused]] auto init_err = writer.init();

    auto entry_point = centipede::EntryPoint<1, 2>{};
    entry_point.set_locals(0.)
        .set_globals(std::pair{ 10U, 0.F }, std::pair{ 11U, 0.F })
        .set_measurement(1.F)
        .set_sigma(1.F);
    auto is_ok = writer.add_entrypoint(entry_point);
    ASSERT_FALSE(is_ok);
    EXPECT_TRUE(is_ok.error() == ErrorCode::writer_entrypoint_rejected);

    auto size = writer.write_current_entry();
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(size.value(), 0);
}

TEST(writer, uninitialized)
{
    auto writer = Binary{ {} };

    auto err = writer.add_entrypoint(valid_entry_point);

    ASSERT_FALSE(err.has_value());
    EXPECT_EQ(err.error(), ErrorCode::writer_uninitialized);

    auto size = writer.write_current_entry();
    ASSERT_FALSE(size.has_value());
    EXPECT_EQ(size.error(), ErrorCode::writer_uninitialized);
}

TEST(writer, read_entrypoint_zero_sigma)
{
    auto writer = Binary{ {} };
    auto init_err = writer.init();
    ASSERT_TRUE(init_err.has_value());

    auto entry_point = centipede::EntryPoint<3, 2>{}
                           .set_locals(1.F, 2.F, 3.F)
                           .set_globals(std::pair{ 10U, 2.F }, std::pair{ 11U, 3.F })
                           .set_measurement(1.F)
                           .set_sigma(-1.F);

    auto err = writer.add_entrypoint(entry_point);
    ASSERT_FALSE(err.has_value());
    EXPECT_TRUE(err.error() == ErrorCode::writer_neg_or_zero_sigma);
}

TEST(writer, read_entrypoint_buffer_overflow)
{
    auto writer = Binary{ { .max_bufferpoint_size = 1 } };
    auto init_err = writer.init();
    ASSERT_TRUE(init_err.has_value());

    auto err = writer.add_entrypoint(valid_entry_point);
    ASSERT_FALSE(err.has_value());
    EXPECT_TRUE(err.error() == ErrorCode::writer_buffer_overflow);
}

TEST(writer, write_current_entry)
{
    auto writer = Binary{ {} };
    auto init_err = writer.init();
    ASSERT_TRUE(init_err.has_value());

    auto err = writer.add_entrypoint(valid_entry_point);

    ASSERT_TRUE(err.has_value());
    auto size = writer.write_current_entry();

    ASSERT_TRUE(size.has_value());
    ASSERT_GT(size.value(), 0);

    const auto& buffer = writer.get_buffer();
    EXPECT_EQ(buffer.first, std::vector{ 0U });
    EXPECT_EQ(buffer.second, std::vector{ 0.F });

    writer.close();

    auto filename = writer.get_config().out_filename;
    ASSERT_TRUE(fs::exists(filename));
    EXPECT_GT(fs::file_size(filename), 0);
}
// NOLINTEND (cppcoreguidelines-avoid-magic-numbers)
// TEST(writer, format) {}
