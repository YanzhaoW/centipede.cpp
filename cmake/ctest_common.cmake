set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/..")

set(CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/build")

# Re-use CDash server details we already have
include(${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake)

if(DEFINED SITE_NAME)
    set(CTEST_SITE ${SITE_NAME})
else()
    site_name(CTEST_SITE)
endif()

if(NOT DEFINED CTEST_BUILD_NAME)
    execute_process(
        COMMAND git config --global user.name
        OUTPUT_VARIABLE output_user_name
    )
    string(STRIP ${output_user_name} output_user_name)
    set(CTEST_BUILD_NAME
        "${CMAKE_HOST_SYSTEM_NAME} local machine from ${output_user_name}"
    )
endif()

set(CTEST_CMAKE_GENERATOR Ninja)
if(NOT DEFINED TEST_MODEL)
    set(TEST_MODEL "Experimental")
endif()

if(NOT DEFINED CTEST_CONFIGURATION_TYPE)
    set(CTEST_CONFIGURATION_TYPE Debug)
endif()

ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})
ctest_start(${TEST_MODEL})
