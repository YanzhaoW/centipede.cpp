set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS
    "verbosity=1:symbolize=1:abort_on_error=1:detect_leaks=1"
)

list(APPEND CMAKE_MODULE_PATH ${CTEST_SCRIPT_DIRECTORY})

include(ctest_common)

set(CONFIGURE_OPTIONS "--preset debug-clang" "-DENABLE_SAN=${ENABLE_SAN}")

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
