#include "centipede/util/error_types.hpp"
#include <magic_enum/magic_enum.hpp>
#include <string_view>

namespace centipede
{
    auto convert_error_type_to_str(ErrorType error_type) -> std::string_view
    {
        return magic_enum::enum_name(error_type);
    }
} // namespace centipede
