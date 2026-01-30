add_subdirectory(centipede)

add_executable(centipede-cli main.cpp)
target_compile_features(centipede-cli PRIVATE cxx_std_23)
target_compile_options(centipede-cli PRIVATE -fno-rtti -fno-exceptions)
target_link_libraries(centipede-cli PRIVATE centipede::centipede)
