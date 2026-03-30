target_sources(
    core
    PRIVATE handler.cpp
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            FILES handler.hpp engines/base_engine.hpp engines/eigen_engine.hpp
)
target_link_libraries(core PRIVATE Eigen3::Eigen)
