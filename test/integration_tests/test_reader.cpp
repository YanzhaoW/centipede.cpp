#include "centipede/reader/binary.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/progress_indicator.hpp"
#include <array>
#include <cstdlib>
#include <indicators/color.hpp>
#include <indicators/font_style.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <print>
#include <ranges>
#include <vector>

auto main() -> int
{
    static_assert(std::ranges::range<centipede::reader::Binary>);
    static_assert(std::ranges::input_range<centipede::reader::Binary>);
    auto reader = centipede::reader::Binary{ centipede::reader::Binary::Config{ .in_filename = "output.bin" } };
    auto init_err = reader.init();
    if (not init_err.has_value())
    {
        std::println(stderr, "Error: {}", init_err.error());
        return EXIT_FAILURE;
    }

    auto progress_adaptor = centipede::progress::ProgressAdaptor{
        centipede::progress::config::BarWidth{ 50 },
        centipede::progress::config::Start{ "[" },
        centipede::progress::config::Fill{ "=" },
        centipede::progress::config::Lead{ ">" },
        centipede::progress::config::Remainder{ " " },
        centipede::progress::config::End{ "]" },
        centipede::progress::config::PostfixText{ "Reading binary data" },
        centipede::progress::config::ForegroundColor{ centipede::progress::ProgressColor::green },
        centipede::progress::config::ShowPercentage{ true },
        centipede::progress::config::FontStyles{
            std::vector<centipede::progress::ProgressFontStyle>{ centipede::progress::ProgressFontStyle::bold } }
    };

    for ([[maybe_unused]] const auto& entry :
         reader | progress_adaptor(reader.get_file_size(), [&reader]() { return reader.get_last_entry_bytes(); }))
    {
    }

    if (not reader.is_ok())
    {
        if (reader.get_status() != centipede::ErrorCode::incomplete)
        {
            std::println(stderr, "Error: {}", reader.get_status());
            return EXIT_FAILURE;
        }
    }

    if (reader.get_n_entries() == 0U)
    {
        std::println(stderr, "Error: no entries read");
        return EXIT_FAILURE;
    }

    if (const auto progress_adaptor_status = progress_adaptor.get_status();
        progress_adaptor_status != centipede::ErrorCode::success)
    {
        std::println(stderr, "Error: {}", progress_adaptor_status);
        return EXIT_FAILURE;
    }

    auto array = std::array{ 1, 2, 3, 4 };

    for ([[maybe_unused]] auto elem : array | progress_adaptor(array.size()))
    {
    }

    if (const auto progress_adaptor_status = progress_adaptor.get_status();
        progress_adaptor_status != centipede::ErrorCode::success)
    {
        std::println(stderr, "Error: {}", progress_adaptor_status);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
