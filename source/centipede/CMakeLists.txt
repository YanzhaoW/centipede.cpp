add_library(core SHARED)
add_library(centipede::centipede ALIAS core)
target_compile_features(core PUBLIC cxx_std_26)
target_compile_options(
    core
    PRIVATE
        -Wall
        -Wconversion
        -Werror
        -Wextra
        -Wshadow
        -fno-exceptions
        -fno-rtti
)

target_sources(
    core
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            BASE_DIRS ${CMAKE_SOURCE_DIR}/source
            FILES centipede.hpp
)

target_link_libraries(core PUBLIC Eigen3::Eigen GSL::gsl)

add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(writer)
add_subdirectory(reader)

if(ENABLE_COVERAGE)
    target_compile_options(core PUBLIC --coverage)
    target_link_options(core PUBLIC --coverage)
endif()

if(ENABLE_SAN STREQUAL "none")
    message(STATUS "All sanitizers are disabled.")
else()
    set(SANITIZER_NAMES "address;undefined;thread")
    if(ENABLE_SAN IN_LIST SANITIZER_NAMES)
        target_compile_options(
            core
            PUBLIC -fno-omit-frame-pointer -fsanitize=${ENABLE_SAN}
        )
        target_link_options(core PUBLIC -fsanitize=${ENABLE_SAN})
    else()
        message(
            FATAL_ERROR
            "ENABLE_SAN is evaluated to \"${ENABLE_SAN}\",\
            which is not in the list \"{${SANITIZER_NAMES}}\""
        )
    endif()
endif()
