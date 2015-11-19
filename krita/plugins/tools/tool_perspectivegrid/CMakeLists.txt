

set(kritatoolperspectivegrid_PART_SRCS
    tool_perspectivegrid.cc
    kis_tool_perspectivegrid.cc
    )
qt5_add_resources(kritatoolperspectivegrid_PART_SRCS tool_perspectivegrid.qrc)
add_library(kritatoolperspectivegrid MODULE ${kritatoolperspectivegrid_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolperspectivegrid kritatoolperspectivegrid.desktop)


target_link_libraries(kritatoolperspectivegrid kritaui)

install(TARGETS kritatoolperspectivegrid  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES
    KisToolPerspectiveGrid.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
