#include "centipede/centipede.hpp"
#include <cstdlib>
#include <vector>

namespace
{
    // void entrypoints_generator() {}
} // namespace

auto main() -> int
{
    auto entries = std::vector<centipede::EntryPoint<>>{};

    auto handler = centipede::Handler<double, { .engine_type = centipede::MatrixEngine::eigen }>{};

    return EXIT_SUCCESS;
}
