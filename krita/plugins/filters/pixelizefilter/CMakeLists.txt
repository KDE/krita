
set(kritapixelizefilter_PART_SRCS kis_pixelize_filter_plugin.cpp kis_pixelize_filter.cpp )


add_library(kritapixelizefilter MODULE ${kritapixelizefilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritapixelizefilter kritapixelizefilter.desktop)


target_link_libraries(kritapixelizefilter kritaui)

install(TARGETS kritapixelizefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapixelizefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


