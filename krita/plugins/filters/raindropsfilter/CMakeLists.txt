
set(kritaraindropsfilter_PART_SRCS kis_raindrops_filter_plugin.cc kis_raindrops_filter.cc )

kde4_automoc(${kritaraindropsfilter_PART_SRCS})

kde4_add_plugin(kritaraindropsfilter ${kritaraindropsfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaraindropsfilter )

target_link_libraries(kritaraindropsfilter kritaui)

install(TARGETS kritaraindropsfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaraindropsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

