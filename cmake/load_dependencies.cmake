# public dependencies:
find_package(Eigen3 REQUIRED CONFIG)
find_package(GSL REQUIRED)

# private dependencies:
find_package(magic_enum REQUIRED CONFIG)
find_package(cxxopts REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(glaze REQUIRED)
find_package(libassert)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
endif()
