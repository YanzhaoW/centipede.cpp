list(APPEND CMAKE_MODULE_PATH ${CTEST_SCRIPT_DIRECTORY})

include(ctest_common)

set(CTEST_COVERAGE_COMMAND "gcov")
set(CONFIGURE_OPTIONS
    "--preset debug-gcc"
    "-DENABLE_COVERAGE=ON"
    ${ADDITIONAL_CONFIGURE_OPTIONS}
)

set(CTEST_COVERAGE_COMMAND "gcovr")

ctest_configure(OPTIONS "${CONFIGURE_OPTIONS}" RETURN_VALUE _return)
ctest_submit(PARTS Configure)

if(_return)
    message(FATAL_ERROR "Configuration failed!")
endif()

ctest_build(RETURN_VALUE _return)
ctest_submit(PARTS Build)

if(_return)
    message(FATAL_ERROR "Build failed!")
endif()

ctest_test(RETURN_VALUE _return)
ctest_submit(PARTS Test)

if(_return)
    message(FATAL_ERROR "Test failed!")
endif()

ctest_coverage(QUIET RETURN_VALUE _return)
ctest_submit(PARTS Coverage)

if(_return)
    message(FATAL_ERROR "Coverage failed!")
endif()
