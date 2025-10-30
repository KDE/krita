if (NOT QT_MAJOR_VERSION STREQUAL "6")
    message(FATAL_ERROR "kritaplatformpluginwayland library is supported with Qt6 only")
endif()

project( kritaplatformpluginwayland )

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
## Define the kritaplatformpluginwayland library
##

set(kritaplatformpluginwayland_LIB_SRCS
    KisWaylandKeyboardWatcher.cpp
    KisExtendedModifiersMapperWayland.cpp
    KritaPlatformPluginWayland.cpp


)

if (KRITA_USE_SURFACE_COLOR_MANAGEMENT_API)
    list(APPEND kritaplatformpluginwayland_LIB_SRCS
         waylandcolormanagement/KisWaylandAPIColorManager.cpp
         waylandcolormanagement/KisWaylandAPIImageDescription.cpp
         waylandcolormanagement/KisWaylandAPIImageDescriptionCreatorParams.cpp
         waylandcolormanagement/KisWaylandAPIOutput.cpp
         waylandcolormanagement/KisWaylandAPISurface.cpp
         waylandcolormanagement/KisWaylandOutputColorInfo.cpp
         waylandcolormanagement/KisWaylandSurfaceColorimetry.cpp
         waylandcolormanagement/KisWaylandSurfaceColorManager.cpp
         waylandcolormanagement/KisWaylandDebugInfoFetcher.cpp)
endif()

kis_add_library(kritaplatformpluginwayland MODULE ${kritaplatformpluginwayland_LIB_SRCS})

set(kritaplatformpluginwayland_WAYLAND_PROTOCOLS
    "${CMAKE_CURRENT_SOURCE_DIR}/protocols/wayland.xml"
)

if (KRITA_USE_SURFACE_COLOR_MANAGEMENT_API)
    list(APPEND kritaplatformpluginwayland_WAYLAND_PROTOCOLS
         "${CMAKE_CURRENT_SOURCE_DIR}/protocols/color-management-v1.xml")
endif()

qt6_generate_wayland_protocol_client_sources(kritaplatformpluginwayland
    FILES
    ${kritaplatformpluginwayland_WAYLAND_PROTOCOLS}
)

set_source_files_properties(
    KisWaylandKeyboardWatcher.cpp
    PROPERTIES
    INCLUDE_DIRECTORIES
    "${Qt6WaylandGlobalPrivate_INCLUDE_DIRS};${Qt6WaylandClient_PRIVATE_INCLUDE_DIRS};${Qt6Gui_PRIVATE_INCLUDE_DIRS}"
    )

target_link_libraries(kritaplatformpluginwayland
    PRIVATE
        Qt::Gui
        Qt::GuiPrivate
        Qt::WaylandClient
        kritaglobal
        kritaui
)

if (KRITA_USE_SURFACE_COLOR_MANAGEMENT_API)
    target_link_libraries(kritaplatformpluginwayland
        PRIVATE
        kritasurfacecolormanagementapi
    )
endif()

install(TARGETS kritaplatformpluginwayland DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

