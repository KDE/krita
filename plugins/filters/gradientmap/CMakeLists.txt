set(kritagradientmap_SOURCES gradientmap.cpp
krita_filter_gradient_map.cpp)

ki18n_wrap_ui(kritagradientmap_SOURCES wdg_gradientmap.ui)

kis_add_library(kritagradientmap MODULE ${kritagradientmap_SOURCES})
target_link_libraries(kritagradientmap kritaui)
install(TARGETS kritagradientmap  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
