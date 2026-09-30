target_sources(
    core
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            FILES
                common_traits.hpp
                eigen_formatter.hpp
                error_types.hpp
                error_code.hpp
                return_types.hpp
    PRIVATE error_types.cpp
)
