set(kritatoolperspectivegrid_SOURCES
    tool_perspectivegrid.cc
    kis_tool_perspectivegrid.cc
    )
qt5_add_resources(kritatoolperspectivegrid_SOURCES tool_perspectivegrid.qrc)
add_library(kritatoolperspectivegrid MODULE ${kritatoolperspectivegrid_SOURCES})

target_link_libraries(kritatoolperspectivegrid kritaui)

install(TARGETS kritatoolperspectivegrid  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES
    KisToolPerspectiveGrid.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
