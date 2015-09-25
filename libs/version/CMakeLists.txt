# generate calligraversion.h
configure_file(calligraversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h)

# generate calligragitversion.h
configure_file(calligragitversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h)


set(koversion_LIB_SRCS
    calligraversion.cpp
    CalligraVersionWrapper.cpp
)

add_library(koversion SHARED ${koversion_LIB_SRCS})
target_link_libraries(koversion Qt5::Core)
generate_export_header(koversion BASE_NAME koversion)

set_target_properties(koversion PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koversion ${INSTALL_TARGETS_DEFAULT_ARGS} )

