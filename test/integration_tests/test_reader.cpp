#include "centipede/reader/binary.hpp"
#include <cstdlib>
#include <print>

auto main() -> int
{
    auto reader = centipede::reader::Binary{ centipede::reader::Binary::Config{ .in_filename = "output.bin" } };
    auto init_err = reader.init();
    if (not init_err.has_value())
    {
        std::println(stderr, "Error: {}", init_err.error());
        return EXIT_FAILURE;
    }

    for ([[maybe_unused]] const auto& entry : reader)
    {
    }

    if (not reader.is_ok())
    {
        std::println(stderr, "Error: {}", reader.get_status());
        return EXIT_FAILURE;
    }

    if (reader.get_n_entries() == 0U)
    {
        std::println(stderr, "Error: no entries read");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
