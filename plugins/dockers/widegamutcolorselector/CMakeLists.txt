set(KRITA_WGCOLORSELECTOR_SOURCES
    WGColorPatches.cpp
    WGColorPreviewPopup.cpp
    WGColorSelectorPlugin.cpp
    WGColorSelectorDock.cpp
    WGShadeSelector.cpp
    WGShadeSlider.cpp
    KisColorSourceToggle.cpp
)

add_library(kritawgcolorselector MODULE ${KRITA_WGCOLORSELECTOR_SOURCES})

target_link_libraries(kritawgcolorselector kritaui)

install(TARGETS kritawgcolorselector DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
