set(kritaartisticcolorselector_SOURCES
    artisticcolorselector_plugin.cpp
    artisticcolorselector_dock.cpp
    kis_color.cpp
    kis_color_selector.cpp
    )

ki18n_wrap_ui(kritaartisticcolorselector_SOURCES 
    forms/wdgArtisticColorSelector.ui 
    forms/wdgARCSSettings.ui
    forms/wdgWheelPreferencesPopup.ui
)

add_library(kritaartisticcolorselector MODULE ${kritaartisticcolorselector_SOURCES})
target_link_libraries(kritaartisticcolorselector kritaui)
install(TARGETS kritaartisticcolorselector DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
