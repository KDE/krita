project(changecase)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} )

set(changecase_SRCS
    ChangecasePlugin.cpp
    Changecase.cpp
    ChangecaseFactory.cpp
)

kde4_add_plugin(changecase ${changecase_SRCS})

target_link_libraries(changecase kotext ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}  ${KDE4_KDECORE_LIBS}  ${KDE4_KDEUI_LIBS})

install(TARGETS changecase DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES changecase.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
