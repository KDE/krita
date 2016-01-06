include(CheckFunctionExists)
check_function_exists(backtrace HAVE_BACKTRACE)
configure_file(config-debug.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-debug.h)

set(kritaglobal_LIB_SRCS
  kis_assert.cpp
  kis_debug.cpp
  kis_shared.cpp
)

add_library(kritaglobal SHARED ${kritaglobal_LIB_SRCS} )
generate_export_header(kritaglobal BASE_NAME kritaglobal)

target_link_libraries(kritaglobal Qt5::Concurrent Qt5::Core Qt5::Gui Qt5::Widgets KF5::I18n)
target_link_libraries(kritaglobal LINK_INTERFACE_LIBRARIES Qt5::Concurrent Qt5::Core Qt5::Gui KF5::I18n)

set_target_properties(kritaglobal PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritaglobal  ${INSTALL_TARGETS_DEFAULT_ARGS})

