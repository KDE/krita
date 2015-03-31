

set(kritatoolgrid_PART_SRCS
    tool_grid.cc
    kis_tool_grid.cc
    )

add_library(kritatoolgrid MODULE ${kritatoolgrid_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolgrid kritatoolgrid.desktop)


target_link_libraries(kritatoolgrid kritaui)

install(TARGETS kritatoolgrid  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_tool_grid.png dark_krita_tool_grid.png light_krita_tool_grid.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolgrid.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolGrid.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
