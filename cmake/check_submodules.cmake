if(NOT EXISTS "${CMAKE_SOURCE_DIR}/util/conan/conan_provider.cmake")
    message(
        FATAL_ERROR
        "\n\
Conan configuration file \"conan_provider.cmake\" doesn't exist!\n\
Did you forget to do:\n\
    git submodule update --init --recursive\n\
"
    )
else()
    message(STATUS "Git submodules has been loaded.")
endif()
