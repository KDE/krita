
set(kritaraindropsfilter_PART_SRCS kis_raindrops_filter_plugin.cc kis_raindrops_filter.cc )


kde4_add_plugin(kritaraindropsfilter ${kritaraindropsfilter_PART_SRCS})



target_link_libraries(kritaraindropsfilter koguiutils kritaimage kritaui ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritaraindropsfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaraindropsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

