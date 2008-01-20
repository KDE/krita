

set(kritatoolgrid_PART_SRCS
    tool_grid.cc
    kis_tool_grid.cc
    )

kde4_add_plugin(kritatoolgrid ${kritatoolgrid_PART_SRCS})



target_link_libraries(kritatoolgrid kritaui)

install(TARGETS kritatoolgrid  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_tool_grid.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolgrid.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


