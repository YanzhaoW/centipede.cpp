enable_testing()
include(GoogleTest)

add_executable(unit_test)

target_sources(
    unit_test
    PRIVATE centipede_unit_test.cpp unit_tests/centipede.cpp
)
target_link_libraries(unit_test PRIVATE GTest::gtest_main centipede::centipede)

gtest_discover_tests(unit_test)
