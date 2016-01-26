set(kritapixelizefilter_SOURCES kis_pixelize_filter_plugin.cpp kis_pixelize_filter.cpp )
add_library(kritapixelizefilter MODULE ${kritapixelizefilter_SOURCES})
target_link_libraries(kritapixelizefilter kritaui)
install(TARGETS kritapixelizefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
