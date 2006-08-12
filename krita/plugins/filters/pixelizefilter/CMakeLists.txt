
set(kritapixelizefilter_PART_SRCS kis_pixelize_filter_plugin.cc kis_pixelize_filter.cc )

kde4_automoc(${kritapixelizefilter_PART_SRCS})

kde4_add_plugin(kritapixelizefilter ${kritapixelizefilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritapixelizefilter )

target_link_libraries(kritapixelizefilter kritaui)

install(TARGETS kritapixelizefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapixelizefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


