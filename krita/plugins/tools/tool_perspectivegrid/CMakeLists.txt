

set(kritatoolperspectivegrid_PART_SRCS
    tool_perspectivegrid.cc
    kis_tool_perspectivegrid.cc
    )

add_library(kritatoolperspectivegrid MODULE ${kritatoolperspectivegrid_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolperspectivegrid kritatoolperspectivegrid.desktop)


target_link_libraries(kritatoolperspectivegrid kritaui)

install(TARGETS kritatoolperspectivegrid  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_perspectivegrid_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolperspectivegrid.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolPerspectiveGrid.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
