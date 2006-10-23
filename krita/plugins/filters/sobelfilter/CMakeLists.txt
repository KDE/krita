




########### next target ###############

set(kritasobelfilter_PART_SRCS kis_sobel_filter_plugin.cc kis_sobel_filter.cc )

kde4_automoc(${kritasobelfilter_PART_SRCS})

kde4_add_plugin(kritasobelfilter ${kritasobelfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritasobelfilter )

target_link_libraries(kritasobelfilter kritaui)

install(TARGETS kritasobelfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasobelfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
