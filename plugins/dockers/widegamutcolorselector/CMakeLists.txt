set(KRITA_WGCOLORSELECTOR_SOURCES
    WGColorPatches.cpp
    WGColorPreviewPopup.cpp
    WGColorSelectorPlugin.cpp
    WGColorSelectorDock.cpp
    WGQuickSettingsWidget.cpp
    WGShadeSelector.cpp
    WGShadeSlider.cpp
    KisColorSourceToggle.cpp
)


ki18n_wrap_ui(KRITA_WGCOLORSELECTOR_SOURCES
    WdgQuickSettings.ui
)

add_library(kritawgcolorselector MODULE ${KRITA_WGCOLORSELECTOR_SOURCES})

target_link_libraries(kritawgcolorselector kritaui)

install(TARGETS kritawgcolorselector DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
