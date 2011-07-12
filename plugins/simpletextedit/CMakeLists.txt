project(SimpleTextEdit)

add_subdirectory( pics )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${KOTEXT_INCLUDES})

set(calligrasimpletextedit_SRCS
    ActionHelper.cpp
    Panel.cpp
    Factory.cpp
    Plugin.cpp
)

kde4_add_ui_files( calligrasimpletextedit_SRCS
    Panel.ui
)

kde4_add_plugin(calligrasimpletextedit ${calligrasimpletextedit_SRCS})

target_link_libraries(calligrasimpletextedit kotext)

install(TARGETS calligrasimpletextedit DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligrasimpletextedit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
