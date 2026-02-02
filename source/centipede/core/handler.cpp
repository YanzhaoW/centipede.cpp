#include "handler.hpp"
#include <spdlog/spdlog.h>

namespace centipede
{
    Handler::Handler() { spdlog::info("centipede handler is launched!"); }
} // namespace centipede
