target_sources(
    core
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            FILES common_traits.hpp error_types.hpp return_types.hpp progress_indicator.hpp
)

target_link_libraries(core PUBLIC indicators::indicators)
