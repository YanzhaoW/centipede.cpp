cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(check_submodules)
include(project_options)

project(centipede LANGUAGES CXX C VERSION 0.0.1)

if(BUILD_DOC_ONLY)
    add_subdirectory(doc)
    return()
endif()

include(load_dependencies)
include(project_config)

add_subdirectory(source)
add_subdirectory(example)
add_subdirectory(test)
if(BUILD_DOC)
    add_subdirectory(doc)
endif()
