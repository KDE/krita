project(changecase)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} )

set(changecase_SRCS
    ChangecasePlugin.cpp
    Changecase.cpp
    ChangecaseFactory.cpp
)

kde4_add_plugin(changecase ${changecase_SRCS})

target_link_libraries(changecase kotext)

install(TARGETS changecase DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES changecase.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
