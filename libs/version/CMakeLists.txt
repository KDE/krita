# generate calligraversion.h
configure_file(calligraversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h)

# generate calligragitversion.h
configure_file(calligragitversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h)


set(koversion_LIB_SRCS
    calligraversion.cpp
)

add_library(koversion SHARED ${koversion_LIB_SRCS})
generate_export_header(koversion BASE_NAME koversion)

set_target_properties(koversion PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koversion ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h
    ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h
    ${CMAKE_CURRENT_BINARY_DIR}/koversion_export.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
