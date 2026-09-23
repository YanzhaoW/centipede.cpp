#include "centipede/centipede.hpp"
#include "centipede/data/ValueError.hpp"
#include "centipede/reader/binary.hpp"
#include "centipede/util/error_types.hpp"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <gtest/gtest.h>
#include <ios>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

using centipede::reader::Binary;
namespace fs = std::filesystem;

namespace centipede::test
{

    TEST(reader, constructor)
    {
        auto reader = Binary{ { .in_filename = "binary_reader_constructor.bin" } };
        EXPECT_FALSE(fs::exists(reader.get_config().in_filename));
    }

    TEST(reader, init)
    {
        auto file_name = std::string{ "binary_reader_init.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto reader = Binary{ { .in_filename = file_name } };
        auto is_ok = reader.init();
        EXPECT_TRUE(is_ok);
    }

    TEST(reader, init_empty_file_name_error)
    {
        auto reader = Binary{ { .in_filename = "" } };
        auto is_ok = reader.init();
        EXPECT_FALSE(is_ok);
        reader = Binary{ { .in_filename = "nonexistent.bin" } };
        is_ok = reader.init();
        EXPECT_FALSE(is_ok);
    }

    TEST(reader, init_nonexisting_file_error)
    {
        auto reader = Binary{ { .in_filename = "nonexistent.bin" } };
        auto is_ok = reader.init();
        EXPECT_FALSE(is_ok);
    }

    TEST(reader, not_initialized)
    {
        auto file_name = std::string{ "not_init.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto is_ok = reader.read_one_entry();
        EXPECT_FALSE(is_ok);
        EXPECT_EQ(is_ok.error(), ErrorCode::reader_uninitialized);
    }

    TEST(reader, file_invalid_idx_size)
    {
        auto file_name = std::string{ "file_invalid_idx_size.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        constexpr auto declared_entry_size = uint32_t{ 2 };
        constexpr auto dummy_data = uint32_t{ 1 };
        // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
        file.write(reinterpret_cast<const char*>(&declared_entry_size), sizeof(declared_entry_size));
        file.write(reinterpret_cast<const char*>(&dummy_data), sizeof(dummy_data));
        // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto is_init_ok = reader.init();
        EXPECT_TRUE(is_init_ok);
        auto is_read_ok = reader.read_one_entry();
        EXPECT_FALSE(is_read_ok);
        EXPECT_EQ(is_read_ok.error(), ErrorCode::reader_file_fail_to_read);
    }

    TEST(reader, file_invalid_val_size)
    {
        auto file_name = std::string{ "file_invalid_val_size.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        constexpr auto declared_entry_size = uint32_t{ 2 };
        // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
        file.write(reinterpret_cast<const char*>(&declared_entry_size), sizeof(declared_entry_size));
        // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        auto read_err = reader.read_one_entry();
        EXPECT_FALSE(read_err);
        EXPECT_EQ(read_err.error(), ErrorCode::reader_file_fail_to_read);
    }

    namespace
    {
        // NOLINTBEGIN
        // (cppcoreguidelines-avoid-magic-numbers)
        auto valid_measurement = ValueError<float>{ 1.F, 1.F };
        auto valid_locals_data = Binary::RawBufferType{ { 1, 2, 3 }, { 1.F, 2.F, 3.F } };
        auto valid_globals_data = Binary::RawBufferType{ { 3, 4, 5 }, { 3.F, 4.F, 5.F } };
        // NOLINTEND
        // (cppcoreguidelines-avoid-magic-numbers)

        auto fill_buffer(Binary::RawBufferType& output,
                         const ValueError<float> measurement,
                         const Binary::RawBufferType& locals_data,
                         const Binary::RawBufferType& globals_data)
        {
            output.first.push_back(uint32_t{ 0 });
            output.second.push_back(measurement.value);
            std::ranges::copy(globals_data.first, std::back_inserter(output.first));
            std::ranges::copy(globals_data.second, std::back_inserter(output.second));

            output.first.push_back(uint32_t{ 0 });
            output.second.push_back(measurement.error);
            std::ranges::copy(locals_data.first, std::back_inserter(output.first));
            std::ranges::copy(locals_data.second, std::back_inserter(output.second));
        }

        auto write_to_file(std::ofstream& file, const Binary::RawBufferType& buffer)
        {
            auto entry_size = static_cast<uint32_t>(buffer.first.size() + buffer.second.size());
            // NOLINTBEGIN (cppcoreguidelines-pro-type-reinterpret-cast)
            file.write(reinterpret_cast<const char*>(&entry_size), sizeof(entry_size));
            file.write(reinterpret_cast<const char*>(buffer.second.data()),
                       static_cast<std::streamsize>(buffer.second.size() * sizeof(float)));
            file.write(reinterpret_cast<const char*>(buffer.first.data()),
                       static_cast<std::streamsize>(buffer.first.size() * sizeof(uint32_t)));
            // NOLINTEND (cppcoreguidelines-pro-type-reinterpret-cast)
        }
    } // namespace

    TEST(reader, valid_single_entry)
    {
        // NOLINTBEGIN(readability-function-cognitive-complexity)
        auto file_name = std::string{ "valid_single_entry.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_globals_data, valid_locals_data);
        fill_buffer(output_buffer, valid_measurement, valid_globals_data, valid_locals_data);
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for (const auto& entry : reader)
        {
            ASSERT_FALSE(entry.empty());
            for (const auto& entrypoint : entry)
            {
                const auto local_data =
                    entrypoint.get_locals() |
                    std::views::transform([](const auto& val_err) -> float { return val_err.value; }) |
                    std::ranges::to<std::vector<float>>();
                EXPECT_EQ(valid_locals_data.second, local_data);
                auto expected_globals = std::views::zip_transform([](const auto& index, const auto& value) -> auto
                                                                  { return std::pair{ index - 1, value }; },
                                                                  valid_globals_data.first,
                                                                  valid_globals_data.second) |
                                        std::ranges::to<std::vector>();
                const auto global_data =
                    entrypoint.get_globals() |
                    std::views::transform([](const auto& idx_val_err)
                                          { return std::pair{ idx_val_err.first, idx_val_err.second.value }; }) |
                    std::ranges::to<std::vector>();
                EXPECT_EQ(expected_globals, global_data);
                EXPECT_EQ(entrypoint.get_measurement(), valid_measurement);
            }
        }
        EXPECT_TRUE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::success);
        reader.close();
        // NOLINTEND(readability-function-cognitive-complexity)
    }

    TEST(reader, reset)
    {
        // NOLINTBEGIN(readability-function-cognitive-complexity)
        auto file_name = std::string{ "reader_reset.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        auto read_err = reader.read_one_entry();
        EXPECT_TRUE(read_err);

        for (const auto& entry : reader)
        {
            for (const auto& entrypoint : entry)
            {
                EXPECT_TRUE(entrypoint.get_globals().empty());
                EXPECT_TRUE(entrypoint.get_locals().empty());
                EXPECT_EQ(entrypoint.get_measurement().value, 0U);
            }
        }
        EXPECT_TRUE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::success);
        reader.close();
        // NOLINTEND(readability-function-cognitive-complexity)
    }

    TEST(reader, invalid_zero_len_entry)
    {

        auto file_name = std::string{ "reader_invalid_zero_len_entry.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_one_len_entry)
    {

        auto file_name = std::string{ "reader_invalid_one_len_entry.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_two_len_entry)
    {

        auto file_name = std::string{ "reader_invalid_two_len_entry.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_three_len_entry)
    {

        auto file_name = std::string{ "reader_invalid_three_len_entry.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_file_begin)
    {
        auto file_name = std::string{ "reader_invalid_file_begin.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 1 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_measurement_chunk)
    {

        auto file_name = std::string{ "invalid_measurement.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        output_buffer.first.at(1) = 1U;
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, invalid_sigma_chunk)
    {
        auto file_name = std::string{ "invalid_sigma.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        // NOLINTBEGIN
        // (cppcoreguidelines-avoid-magic-numbers)
        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 1 });

        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 2 });

        output_buffer.first.push_back(uint32_t{ 1 });
        output_buffer.second.push_back(float{ 3 });

        output_buffer.first.push_back(uint32_t{ 2 });
        output_buffer.second.push_back(float{ 4 });

        output_buffer.first.push_back(uint32_t{ 0 });
        output_buffer.second.push_back(float{ 5 });
        // NOLINTEND
        // (cppcoreguidelines-avoid-magic-numbers)
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, file_resize_buffer)
    {
        auto file_name = std::string{ "file_resize_buffer.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name, .max_bufferpoint_size = 1U } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        auto read_err = reader.read_one_entry();
        EXPECT_TRUE(read_err);
    }

    TEST(reader, end_of_file)
    {
        auto file_name = std::string{ "reader_end_of_file.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };
        auto output_buffer = Binary::RawBufferType{ { uint32_t{ 0 } }, { 0.F } };
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        fill_buffer(output_buffer, valid_measurement, valid_locals_data, valid_globals_data);
        write_to_file(file, output_buffer);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        EXPECT_TRUE(init_err);
        auto read_err = reader.read_one_entry();
        EXPECT_TRUE(read_err);
        read_err = reader.read_one_entry();
        EXPECT_TRUE(read_err);
        EXPECT_TRUE(reader.is_end_of_file());
        EXPECT_EQ(read_err.value(), 0U);
    }

    TEST(reader, incomplete_header_error)
    {
        auto file_name = std::string{ "reader_incomplete_header_error.bin" };
        auto file = std::ofstream{ file_name, std::ios::out | std::ios::binary | std::ios::trunc };

        constexpr auto invalid_header = char{ 42 };
        file.write(&invalid_header, 1);
        file.close();
        auto reader = Binary{ { .in_filename = file_name } };
        auto init_err = reader.init();
        ASSERT_TRUE(init_err);
        for ([[maybe_unused]] const auto& entry : reader)
        {
        }
        EXPECT_EQ(reader.get_n_entries(), 0U);
        EXPECT_FALSE(reader.is_ok());
        EXPECT_EQ(reader.get_status(), ErrorCode::reader_file_fail_to_read);
        reader.close();
    }

    TEST(reader, entry_value_check)
    {
        const auto filename = "test_entry.bin";
        auto writer = writer::Binary{ { .out_filename = filename } };
        auto reader = reader::Binary{ { .in_filename = filename } };

        auto is_writer_init_ok = writer.init();
        ASSERT_TRUE(is_writer_init_ok);

        const auto entrypoint =
            EntryPoint{}.add_global(4, 2.3).add_global(15, 2.0).add_local(2.0).add_local(1.0).set_measurement(
                ValueError{ 3.4F, 2.1F });
        auto is_add_ok = writer.add_entrypoint(entrypoint);
        ASSERT_TRUE(is_add_ok);

        const auto raw_writer_buffer = writer.get_buffer();
        auto is_write_ok = writer.write_current_entry();
        ASSERT_TRUE(is_write_ok);
        writer.close();

        auto is_reader_init_ok = reader.init();
        auto size_res = reader.read_one_entry();
        ASSERT_TRUE(size_res);

        auto current_entries = reader.get_current_entry();
        EXPECT_EQ(current_entries.size(), 1);

        const auto reader_raw_entry_buffer = reader.get_buffer();

        EXPECT_EQ(raw_writer_buffer.first, reader_raw_entry_buffer.first);
        EXPECT_EQ(raw_writer_buffer.second, reader_raw_entry_buffer.second);

        EXPECT_EQ(current_entries.front(), entrypoint)
            << std::format("write entry: {}\nread entry: {}\nwrite buffer: {}\nread buffer: {}",
                           entrypoint,
                           current_entries.front(),
                           raw_writer_buffer,
                           reader_raw_entry_buffer);
    }
} // namespace centipede::test
