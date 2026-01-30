set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS
    "verbosity=1:symbolize=1:abort_on_error=1:detect_leaks=1"
)

list(APPEND CMAKE_MODULE_PATH ${CTEST_SCRIPT_DIRECTORY})

include(ctest_common)

set(CONFIGURE_OPTIONS "--preset debug-clang" "-DENABLE_SAN=${ENABLE_SAN}")

ctest_configure(OPTIONS "${CONFIGURE_OPTIONS}")

ctest_submit(PARTS Configure)

ctest_build()

ctest_submit(PARTS Build)

ctest_memcheck(RETURN_VALUE _ctest_test_ret_val)

ctest_submit(PARTS MemCheck)

if(_ctest_test_ret_val)
    message(
        FATAL_ERROR
        "\
::error title=${ENABLE_SAN} sanitizer check failed!::Please go to\
the dashboard (https://my.cdash.org/index.php?project=centipede-projekt) to see more details!\
"
    )
endif()
