# generate calligraversion.h
configure_file(calligraversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h)

# generate calligragitversion.h
configure_file(calligragitversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h)

include_directories( ${QT_INCLUDES})

set(koversion_LIB_SRCS
    calligraversion.cpp
)

kde4_add_library(koversion SHARED ${koversion_LIB_SRCS})

target_link_libraries(koversion ${QT_QTCORE_LIBRARY} )

target_link_libraries(koversion LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} )

set_target_properties(koversion PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koversion ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/calligraversion.h
    ${CMAKE_CURRENT_BINARY_DIR}/calligragitversion.h
    koversion_export.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
