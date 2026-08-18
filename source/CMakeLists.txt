set(common_compile_flags
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wshadow
    -fno-exceptions
    -fno-rtti
)

add_subdirectory(centipede)

add_executable(exe cli.cpp)
target_compile_options(exe PRIVATE ${common_compile_flags})
target_compile_features(exe PRIVATE cxx_std_26)
set_target_properties(exe PROPERTIES OUTPUT_NAME centipede)
target_compile_definitions(exe PRIVATE CXXOPTS_NO_EXCEPTIONS)
target_link_libraries(exe PRIVATE centipede::cli)
