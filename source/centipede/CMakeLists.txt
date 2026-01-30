target_sources(
    core
    PRIVATE centipede.cpp
    PUBLIC FILE_SET publicHeaders TYPE HEADERS FILES centipede.hpp
)
