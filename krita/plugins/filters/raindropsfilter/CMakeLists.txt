
set(kritaraindropsfilter_PART_SRCS kis_raindrops_filter_plugin.cpp kis_raindrops_filter.cpp )


add_library(kritaraindropsfilter MODULE ${kritaraindropsfilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritaraindropsfilter kritaraindropsfilter.desktop)



target_link_libraries(kritaraindropsfilter kritaui)

install(TARGETS kritaraindropsfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
