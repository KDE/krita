# generate calligraversion.h
configure_file(calligraversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h)

# generate calligragitversion.h
configure_file(calligragitversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h)

set(kritaversion_LIB_SRCS
    calligraversion.cpp
    KritaVersionWrapper.cpp
)

add_library(kritaversion SHARED ${kritaversion_LIB_SRCS})
target_link_libraries(kritaversion Qt5::Core)
generate_export_header(kritaversion BASE_NAME kritaversion)

set_target_properties(kritaversion PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaversion ${INSTALL_TARGETS_DEFAULT_ARGS} )

