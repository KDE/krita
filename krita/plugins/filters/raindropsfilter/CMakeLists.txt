
set(kritaraindropsfilter_PART_SRCS kis_raindrops_filter_plugin.cpp kis_raindrops_filter.cpp )


kde4_add_plugin(kritaraindropsfilter ${kritaraindropsfilter_PART_SRCS})



target_link_libraries(kritaraindropsfilter kritaui)

install(TARGETS kritaraindropsfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaraindropsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

