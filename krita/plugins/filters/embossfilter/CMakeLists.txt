
set(kritaembossfilter_PART_SRCS kis_emboss_filter_plugin.cpp kis_emboss_filter.cpp )


add_library(kritaembossfilter MODULE ${kritaembossfilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritaembossfilter kritaembossfilter.desktop)


target_link_libraries(kritaembossfilter kritaui)

install(TARGETS kritaembossfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
