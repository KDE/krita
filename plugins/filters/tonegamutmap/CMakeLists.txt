set(kritatonegamutmapfilter_SOURCES KisToneGamutMapFilter.cpp )


kis_add_library(kritatonegamutmapfilter MODULE ${kritatonegamutmapfilter_SOURCES})
target_link_libraries(kritatonegamutmapfilter kritaui)
install(TARGETS kritatonegamutmapfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

