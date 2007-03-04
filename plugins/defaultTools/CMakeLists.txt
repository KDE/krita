project( defaulttools )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOFFICECORE_INCLUDES} )

########### Flake Plugin library ###############

SET ( defaulttools_SRCS
    Plugin.cpp
    ConnectionTool.cpp
    ConnectionToolFactory.cpp
)

#kde4_add_ui_files(defaulttools_SRCS
#)

kde4_automoc(${defaulttools_SRCS})
kde4_add_plugin(defaulttools ${defaulttools_SRCS})
target_link_libraries(defaulttools flake kofficeui kotext)
install(TARGETS defaulttools DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES defaulttools.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES tool_zoom_plus_cursor.png tool_zoom_minus_cursor.png tool_zoom.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons)
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
