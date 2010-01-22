

set(kritatoolperspectivegrid_PART_SRCS
    tool_perspectivegrid.cc
    kis_tool_perspectivegrid.cc
    )

kde4_add_plugin(kritatoolperspectivegrid ${kritatoolperspectivegrid_PART_SRCS})



target_link_libraries(kritatoolperspectivegrid kritaui)

install(TARGETS kritatoolperspectivegrid  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_perspectivegrid.png tool_perspectivegrid_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolperspectivegrid.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


