set(kritagradientmap_SOURCES gradientmap.cpp krita_gradient_map_color_transformation.cpp krita_filter_gradient_map.cpp)

ki18n_wrap_ui(kritagradientmap_SOURCES wdg_gradientmap.ui)

add_library(kritagradientmap MODULE ${kritagradientmap_SOURCES})
target_link_libraries(kritagradientmap kritaui)
install(TARGETS kritagradientmap  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
