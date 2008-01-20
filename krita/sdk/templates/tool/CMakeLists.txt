

set(kritatool%{APPNAMELC}_PART_SRCS
    tool_%{APPNAMELC}.cc
    kis_tool_%{APPNAMELC}.cc
    )

kde4_add_plugin(kritatool%{APPNAMELC} ${kritatool%{APPNAMELC}_PART_SRCS})



target_link_libraries(kritatool%{APPNAMELC} kritaui)

install(TARGETS kritatool%{APPNAMELC}  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

# install( FILES  tool_%{APPNAMELC}.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatool%{APPNAMELC}.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


