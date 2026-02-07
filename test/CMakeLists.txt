enable_testing()
include(GoogleTest)

add_executable(unit_test)

add_subdirectory(unit_tests)

target_sources(unit_test PRIVATE centipede_unit_test.cpp)

target_link_libraries(unit_test PRIVATE GTest::gtest_main centipede::centipede magic_enum::magic_enum)

gtest_discover_tests(unit_test)

add_subdirectory(integration_tests)
