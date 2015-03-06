
set(kritaartisticcolorselector_PART_SRCS
    artisticcolorselector_plugin.cpp
    artisticcolorselector_dock.cpp
    kis_color.cpp
    kis_color_selector.cpp
)

ki18n_wrap_ui(kritaartisticcolorselector_PART_SRCS forms/wdgArtisticColorSelector.ui forms/wdgColorPreferencesPopup.ui)
add_library(kritaartisticcolorselector MODULE ${kritaartisticcolorselector_PART_SRCS})

target_link_libraries(kritaartisticcolorselector kritaui)

install(TARGETS kritaartisticcolorselector DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES krita_artisticcolorselector.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
