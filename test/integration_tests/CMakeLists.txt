set(COMPILE_OPTIONS
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wshadow
    -fno-exceptions
    -fno-rtti
)

add_executable(integration_test_writer test_writer.cpp)

target_link_libraries(integration_test_writer PRIVATE centipede::centipede)

target_compile_options(integration_test_writer PRIVATE ${COMPILE_OPTIONS})

# if(ENABLE_COVERAGE)
#     target_compile_options(integration_test_writer PRIVATE --coverage)
#     target_link_options(integration_test_writer PRIVATE --coverage)
# endif()

add_test(NAME integration_test_writer_data_gen COMMAND integration_test_writer)
set_tests_properties(integration_test_writer_data_gen PROPERTIES TIMEOUT 10)

add_test(
    NAME integration_test_writer_res
    COMMAND
        bash -c
        "wget \"https://raw.githubusercontent.com/YanzhaoW/Millepede-II/refs/heads/cmake/tools/readMilleBinary.py\" \&\& python3 readMilleBinary.py --quiet \"output.bin\""
)

set_tests_properties(
    integration_test_writer_res
    PROPERTIES DEPENDS integration_test_writer_data_gen
)
