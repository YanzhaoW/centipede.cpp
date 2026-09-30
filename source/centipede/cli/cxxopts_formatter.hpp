#pragma once

#include "centipede/util/common_definitions.hpp"
#include <ios>
#include <istream>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>

namespace spdlog::level
{
    inline auto operator>>(std::istream& in_stream, level_enum& level) -> std::istream&
    {
        std::string level_str{};
        in_stream >> level_str;

        auto input_level_enum = magic_enum::enum_cast<spdlog::level::level_enum>(level_str);
        if (not input_level_enum)
        {
            spdlog::error(
                "Unknown log level {:?}. Available values: {}", level_str, magic_enum::enum_names<level_enum>());
            in_stream.setstate(std::ios::failbit);
        }
        else
        {
            level = input_level_enum.value();
        }

        return in_stream;
    }
} // namespace spdlog::level

namespace centipede::common
{
    inline auto operator>>(std::istream& in_stream, IO& io) -> std::istream&
    {
        std::string io_str{};
        in_stream >> io_str;

        auto input_level_enum = magic_enum::enum_cast<IO>(io_str);
        if (not input_level_enum)
        {
            spdlog::error("Unknown IO {:?}. Available IOs: {}", io_str, magic_enum::enum_names<IO>());
            in_stream.setstate(std::ios::failbit);
        }
        else
        {
            io = input_level_enum.value();
        }

        return in_stream;
    }
} // namespace centipede::common
