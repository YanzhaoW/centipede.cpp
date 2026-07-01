#include "centipede/reader/binary.hpp"
#include "centipede/util/error_types.hpp"
#include "centipede/util/progress_indicator.hpp"
#include <array>
#include <cstdlib>
#include <print>
#include <ranges>

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

    auto progress_adaptor = centipede::progress::ProgressAdaptor{};
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

    auto array = std::array{ 1, 2, 3, 4 };

    for ([[maybe_unused]] auto elem : array | progress_adaptor(array.size()))
    {
    }

    return EXIT_SUCCESS;
}
