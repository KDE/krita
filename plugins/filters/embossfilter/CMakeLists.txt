set(kritaembossfilter_SOURCES kis_emboss_filter_plugin.cpp kis_emboss_filter.cpp )
kis_add_library(kritaembossfilter MODULE ${kritaembossfilter_SOURCES})
target_link_libraries(kritaembossfilter kritaui)
install(TARGETS kritaembossfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
