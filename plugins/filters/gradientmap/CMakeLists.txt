set(kritagradientmap_SOURCES
    KisGradientMapFilter.cpp
    KisGradientMapConfigWidget.cpp
    KisGradientMapFilterConfiguration.cpp
)

ki18n_wrap_ui(kritagradientmap_SOURCES
    KisGradientMapConfigWidget.ui
)

add_library(kritagradientmap MODULE ${kritagradientmap_SOURCES})
target_link_libraries(kritagradientmap kritaui)
install(TARGETS kritagradientmap  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
