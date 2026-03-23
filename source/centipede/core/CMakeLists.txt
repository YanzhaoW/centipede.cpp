target_sources(
    core
    PRIVATE handler.cpp
    PUBLIC FILE_SET publicHeaders TYPE HEADERS FILES handler.hpp
)
target_link_libraries(core PRIVATE Eigen3::Eigen)
