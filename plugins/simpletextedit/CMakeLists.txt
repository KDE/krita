project(SimpleTextEdit)

add_subdirectory( pics )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${KOTEXT_INCLUDES})

set(kofficesimpletextedit_SRCS
    ActionHelper.cpp
    Panel.cpp
    Factory.cpp
    Plugin.cpp
)

kde4_add_ui_files( kofficesimpletextedit_SRCS
    Panel.ui
)

kde4_add_plugin(kofficesimpletextedit ${kofficesimpletextedit_SRCS})

target_link_libraries(kofficesimpletextedit kotext)

install(TARGETS kofficesimpletextedit DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficesimpletextedit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
