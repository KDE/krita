project(changecase)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} )

set(changecase_SRCS
    ChangecasePlugin.cpp
    Changecase.cpp
    ChangecaseFactory.cpp
)

kde4_add_plugin(calligra_textediting_changecase ${changecase_SRCS})

target_link_libraries(calligra_textediting_changecase kotext)

install(TARGETS calligra_textediting_changecase DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES changecase.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
