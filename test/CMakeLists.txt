set(COMPILE_OPTIONS
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wshadow
    -fno-exceptions
    -fno-rtti
)

include(GoogleTest)

add_executable(unit_test)

add_subdirectory(unit_tests)

target_sources(unit_test PRIVATE centipede_unit_test.cpp)

target_compile_options(unit_test PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(
    unit_test
    PRIVATE gtest::gtest centipede::centipede magic_enum::magic_enum
)

gtest_discover_tests(unit_test)

add_subdirectory(integration_tests)
