target_sources(
    core
    PRIVATE binary.cpp
    PUBLIC FILE_SET publicHeaders TYPE HEADERS FILES binary.hpp
)
