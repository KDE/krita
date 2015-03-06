
set(kritatooldyna_PART_SRCS
    tool_dyna.cpp
    kis_tool_dyna.cpp
    )


add_library(kritatooldyna MODULE ${kritatooldyna_PART_SRCS})


target_link_libraries(kritatooldyna kritaui)

install(TARGETS kritatooldyna  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

# TODO
install( FILES  krita_tool_dyna.png dark_krita_tool_dyna.png light_krita_tool_dyna.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatooldyna.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


