# generate kritaversion.h
configure_file(kritaversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kritaversion.h)

# generate kritagitversion.h
configure_file(kritagitversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kritagitversion.h)

set(kritaversion_LIB_SRCS
    KritaVersionWrapper.cpp
)

add_library(kritaversion SHARED ${kritaversion_LIB_SRCS})
target_link_libraries(kritaversion Qt5::Core)
generate_export_header(kritaversion BASE_NAME kritaversion)

set_target_properties(kritaversion PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaversion ${INSTALL_TARGETS_DEFAULT_ARGS} )

