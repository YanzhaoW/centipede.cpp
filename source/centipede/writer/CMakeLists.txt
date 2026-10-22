target_sources(
    core
    PRIVATE binary.cpp
    PUBLIC FILE_SET publicHeaders TYPE HEADERS FILES binary.hpp
)

if(CapnProto_FOUND)
    set(CAPNPC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/source")

    file(MAKE_DIRECTORY "${CAPNPC_OUTPUT_DIR}")

    capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS entry.capnp)
    target_sources(
        core
        PRIVATE capnproto.cpp ${CAPNP_SRCS}
        PUBLIC FILE_SET publicHeaders TYPE HEADERS FILES capnproto.hpp
        PRIVATE FILE_SET privateHeaders TYPE HEADERS FILES ${CAPNP_HDRS}
    )
    target_link_libraries(core PUBLIC capnproto::capnproto)

    # TODO: capnproto will implement target integration with codegen
    add_custom_target(generate_entry_capnp DEPENDS ${CAPNP_SRCS} ${CAPNP_HDRS})
    add_dependencies(core generate_entry_capnp)
endif()
