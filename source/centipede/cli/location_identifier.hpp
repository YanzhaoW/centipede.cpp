#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#if defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace centipede::cli
{
    constexpr auto init_path_size = 100;

    inline auto get_current_exe_location() -> std::expected<std::filesystem::path, std::string>
    {
        auto exe_str = std::string{};
        auto exe_path_size = uint32_t{ init_path_size };
        exe_str.resize(exe_path_size);
#if defined(__APPLE__)
        while (_NSGetExecutablePath(exe_str.data(), &exe_path_size) != 0)
        {
            if (exe_path_size != exe_str.size())
            {
                exe_str.resize(exe_path_size);
            }
            else
            {
                return std::unexpected{ std::string{ "Failed to retrieve the location of the current executable!" } };
            }
        }

#elif defined(__linux__)
        while (true)
        {
            auto size = readlink("/proc/self/exe", exe_str.data(), exe_str.size());
            if (size == -1)
            {
                return std::unexpected{ std::string{ "Failed to retrieve the location of the current executable!" } };
            }
            if (size < exe_str.size())
            {
                exe_path_size = size;
                break;
            }
            exe_str.resize(size * 2);
        }

#endif
        auto err = std::error_code{};
        auto full_path = std::filesystem::weakly_canonical(
            std::filesystem::path{ std::string_view{ exe_str.data(), exe_path_size } }, err);

        if (err)
        {
            return std::unexpected{ err.message() };
        }
        return full_path;
    }
} // namespace centipede::cli
