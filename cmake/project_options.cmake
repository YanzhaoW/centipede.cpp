option(ENABLE_TEST "Enable testing framework of the project." ON)
option(BUILD_DOC "Build the documentation for this project." OFF)
option(BUILD_DOC_ONLY "Only build the documentation for this project." OFF)
option(ENABLE_COVERAGE "Enable coverage flags" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy checks" OFF)

# for sanitizers
set(ENABLE_SAN "none" CACHE STRING "Enable one of sanitizers")

if(ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        clang-tidy
        --header-filter=${CMAKE_SOURCE_DIR}/source.*
        --allow-no-checks
        --format-style="file"
        --warnings-as-errors=*
        # --exclude-header-filter=.*\.conan2.*
        --use-color
        -p=${CMAKE_BINARY_DIR}
    )
endif()

# set the cmake variables for the communication with conan
set(ENV{CMAKE_ENABLE_TEST} ${ENABLE_TEST})
