project(MetaData)

#add_subdirectory( pics )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${KOTEXT_INCLUDES})

set(kofficevariableedit_SRCS
    ActionHelper.cpp
    Panel.cpp
    Factory.cpp
    Plugin.cpp
)

kde4_add_ui_files( kofficevariableedit_SRCS
    Panel.ui
)

kde4_add_plugin(kofficevariableedit ${kofficevariableedit_SRCS})

target_link_libraries(kofficevariableedit kotext)

install(TARGETS kofficevariableedit DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficevariableedit.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
