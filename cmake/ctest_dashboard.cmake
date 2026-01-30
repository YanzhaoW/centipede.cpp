list(APPEND CMAKE_MODULE_PATH ${CTEST_SCRIPT_DIRECTORY})

include(ctest_common)

set(CONFIGURE_OPTIONS
    "--preset ${CONFIGURE_PRESET}"
    ${ADDITIONAL_CONFIGURE_OPTIONS}
)

ctest_configure(OPTIONS "${CONFIGURE_OPTIONS}")

ctest_submit(PARTS Configure)

ctest_build()

ctest_submit(PARTS Build)

ctest_test(RETURN_VALUE _ctest_test_ret_val)

ctest_submit(PARTS Test)

if(_ctest_test_ret_val)
    message(FATAL_ERROR "Some tests failed!")
else()
    message(STATUS "All tests passed!")
endif()
