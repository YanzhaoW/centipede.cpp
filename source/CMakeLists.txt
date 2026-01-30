add_library(core SHARED)
add_library(centipede::centipede ALIAS core)
target_compile_features(core PRIVATE cxx_std_23)
target_compile_options(core PRIVATE -fno-rtti -fno-exceptions)

target_sources(
    core
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(centipede)

add_executable(centipede-cli main.cpp)
target_compile_features(centipede-cli PRIVATE cxx_std_23)
target_compile_options(centipede-cli PRIVATE -fno-rtti -fno-exceptions)
