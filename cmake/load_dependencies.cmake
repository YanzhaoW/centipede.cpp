find_package(spdlog REQUIRED CONFIG)
find_package(magic_enum REQUIRED CONFIG)
find_package(CLI11 REQUIRED CONFIG)

if(ENABLE_TEST)
    find_package(GTest CONFIG REQUIRED)
endif()
