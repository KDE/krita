set(kritagradientmap_SOURCES
    KisGradientMapFilter.cpp
    KisGradientMapFilterConfigWidget.cpp
    KisGradientMapFilterConfiguration.cpp
    KisGradientMapFilterPlugin.cpp
    KisGradientMapFilterNearestCachedGradient.cpp
    KisGradientMapFilterDitherCachedGradient.cpp
)

ki18n_wrap_ui(kritagradientmap_SOURCES
    KisGradientMapFilterConfigWidget.ui
)

add_library(kritagradientmap MODULE ${kritagradientmap_SOURCES})
target_link_libraries(kritagradientmap kritaui)
install(TARGETS kritagradientmap  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
