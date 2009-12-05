project(%{APPNAME})

cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include(KDE4Defaults)
include(MacroLibrary)
include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} )

set(%{APPNAMELC}_SRCS
    %{APPNAME}Plugin.cpp
    %{APPNAME}.cpp
    %{APPNAME}Factory.cpp
)

kde4_add_plugin(kotext%{APPNAMELC} ${%{APPNAMELC}_SRCS})

target_link_libraries(kotext%{APPNAMELC}
    QtCore
    QtGui
    kotext
    ${KDE4_KDEUI_LIBS}
)

install(TARGETS kotext%{APPNAMELC} DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kotext%{APPNAMELC}.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
