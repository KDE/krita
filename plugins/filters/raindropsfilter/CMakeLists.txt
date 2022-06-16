set(kritaraindropsfilter_SOURCES kis_raindrops_filter_plugin.cpp kis_raindrops_filter.cpp )
kis_add_library(kritaraindropsfilter MODULE ${kritaraindropsfilter_SOURCES} PCH_FILE kis_image_pch.h)
target_link_libraries(kritaraindropsfilter kritaui)
install(TARGETS kritaraindropsfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
