list(APPEND CMAKE_MODULE_PATH ${CTEST_SCRIPT_DIRECTORY})

include(ctest_common)

set(CTEST_COVERAGE_COMMAND "gcov")
set(CONFIGURE_OPTIONS
    "--preset debug-gcc"
    "-DENABLE_COVERAGE=ON"
    ${ADDITIONAL_CONFIGURE_OPTIONS}
)
set(CTEST_COVERAGE_EXTRA_FLAGS
    "--object-directory ${CTEST_BINARY_DIRECTORY}/source"
)

ctest_configure(OPTIONS "${CONFIGURE_OPTIONS}")
ctest_submit(PARTS Configure)

ctest_build()
ctest_submit(PARTS Build)

ctest_test()
ctest_submit(PARTS Test)

ctest_coverage(QUIET)
ctest_submit(PARTS Coverage)
