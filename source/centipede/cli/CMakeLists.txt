add_library(cli STATIC)

add_library(centipede::cli ALIAS cli)
target_sources(
    cli
    PUBLIC
        FILE_SET privateHeaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_SOURCE_DIR}/source
            FILES
                application.hpp
                config.hpp
                location_identifier.hpp
                lua_connector.hpp
                spdlog_stream.hpp
    PRIVATE application.cpp lua_connector.cpp
)

target_compile_definitions(cli PUBLIC SOL_NO_EXCEPTIONS)

target_link_libraries(
    cli
    PUBLIC
        core
        cxxopts::cxxopts
        glaze::glaze
        magic_enum::magic_enum
        sol2::sol2
        spdlog::spdlog
)
