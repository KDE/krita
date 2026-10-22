set(kritatonegamutmapfilter_SOURCES KisToneGamutMapFilter.cpp )

ki18n_wrap_ui(kritatonegamutmapfilter_SOURCES
    kistonegamutmapwidget.ui
    )

kis_add_library(kritatonegamutmapfilter MODULE ${kritatonegamutmapfilter_SOURCES})
target_link_libraries(kritatonegamutmapfilter kritaui)
install(TARGETS kritatonegamutmapfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

