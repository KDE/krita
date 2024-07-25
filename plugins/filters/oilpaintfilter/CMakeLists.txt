set(kritaoilpaintfilter_SOURCES kis_oilpaint_filter_plugin.cpp kis_oilpaint_filter.cpp )
kis_add_library(kritaoilpaintfilter MODULE ${kritaoilpaintfilter_SOURCES})
target_link_libraries(kritaoilpaintfilter kritaui)
install(TARGETS kritaoilpaintfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
