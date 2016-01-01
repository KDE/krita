set(kritaartisticcolorselector_SOURCES
    artisticcolorselector_plugin.cpp
    artisticcolorselector_dock.cpp
    kis_color.cpp
    kis_color_selector.cpp
)

ki18n_wrap_ui(kritaartisticcolorselector_SOURCES 
    forms/wdgArtisticColorSelector.ui 
    forms/wdgColorPreferencesPopup.ui
)

add_library(kritaartisticcolorselector MODULE ${kritaartisticcolorselector_SOURCES})
target_link_libraries(kritaartisticcolorselector kritaui)
install(TARGETS kritaartisticcolorselector DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
