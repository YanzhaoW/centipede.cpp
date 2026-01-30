add_library(core SHARED)
add_library(centipede::centipede ALIAS core)

target_sources(
    core
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(centipede)

add_executable(centipede-cli main.cpp)
