find_package(mps)

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
    PRIVATE
        centipede::centipede
        gtest::gtest
        magic_enum::magic_enum
        spdlog::spdlog
)

gtest_discover_tests(unit_test)

add_subdirectory(integration_tests)

if(mps_FOUND)
    add_executable(generator)

    target_sources(generator PRIVATE generator.cpp)
    target_compile_options(generator PRIVATE ${COMPILE_OPTIONS})
    target_link_libraries(
        generator
        PRIVATE
            mps::mps
            centipede::centipede
            magic_enum::magic_enum
            spdlog::spdlog
            cxxopts::cxxopts
            glaze::glaze
    )
    target_compile_definitions(generator PRIVATE CXXOPTS_NO_EXCEPTIONS)
endif()
