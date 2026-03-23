target_sources(
    core
    PRIVATE handler.cpp
    PUBLIC
        FILE_SET publicHeaders
            TYPE HEADERS
            FILES
                engines/base_engine.hpp
                engines/eigen_engine.hpp
                engines/master_engine.hpp
                handler.hpp
)
target_link_libraries(core PUBLIC Eigen3::Eigen GSL::gsl)
target_compile_definitions(
    core
    PUBLIC
        # EIGEN_MATRIXBASE_PLUGIN="${CMAKE_CURRENT_SOURCE_DIR}/engines/eigen_matrix_base_addon.inc"
        EIGEN_RUNTIME_NO_MALLOC
)
