




########### next target ###############

set(kritasobelfilter_PART_SRCS kis_sobel_filter_plugin.cpp kis_sobel_filter.cpp )


kde4_add_plugin(kritasobelfilter ${kritasobelfilter_PART_SRCS})



target_link_libraries(kritasobelfilter kritaui)

install(TARGETS kritasobelfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasobelfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
