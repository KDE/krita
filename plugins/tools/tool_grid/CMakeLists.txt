set(kritatoolgrid_SOURCES
    tool_grid.cc
    kis_tool_grid.cc
    )

add_library(kritatoolgrid MODULE ${kritatoolgrid_SOURCES})

target_link_libraries(kritatoolgrid kritaui)

install(TARGETS kritatoolgrid  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############
install( FILES
    KisToolGrid.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
