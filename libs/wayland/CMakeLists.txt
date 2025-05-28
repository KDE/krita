if (NOT QT_MAJOR_VERSION STREQUAL "6")
    message(FATAL_ERROR "kritawayland library is supported with Qt6 only")
endif()

##
## Check for the presence of custom Wayland patches in Qt
##

set(CMAKE_REQUIRED_INCLUDES ${Qt6Gui_INCLUDE_DIRS} ${Qt6WaylandClient_PRIVATE_INCLUDE_DIRS} ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${Qt6Gui_LIBRARIES} ${Qt6WaylandClient_LIBRARIES})

check_cxx_source_compiles("
#include <QGuiApplication>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

int main(int, char *[]) {
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto xkbContext = waylandApp->xkbContext();
    Q_UNUSED(xkbContext);
    return 0;
}
"
    KRITA_QT_HAS_XKB_CONTEXT_IN_NATIVE_INTERFACE
    )

unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)

configure_file(config-qt-wayland-patches-present.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-qt-wayland-patches-present.h)

##
## Define the kritawayland library
##

set(kritawayland_LIB_SRCS
    KisWaylandKeyboardWatcher.cpp
)

kis_add_library(kritawayland SHARED ${kritawayland_LIB_SRCS})

qt6_generate_wayland_protocol_client_sources(kritawayland
    FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/protocols/wayland.xml"
)

set_source_files_properties(
    KisWaylandKeyboardWatcher.cpp
    PROPERTIES
    INCLUDE_DIRECTORIES
    "${Qt6WaylandGlobalPrivate_INCLUDE_DIRS};${Qt6WaylandClient_PRIVATE_INCLUDE_DIRS};${Qt6Gui_PRIVATE_INCLUDE_DIRS}"
    )

target_link_libraries(kritawayland
    PRIVATE
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClient
        kritaglobal
    )

generate_export_header(kritawayland BASE_NAME kritawayland)

set_target_properties(kritawayland PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritawayland ${INSTALL_TARGETS_DEFAULT_ARGS} )

