set(KRITA_WGCOLORSELECTOR_SOURCES
    WGActionManager.cpp
    WGColorPatches.cpp
    WGColorPreviewToolTip.cpp
    WGColorSelectorDock.cpp
    WGColorSelectorPlugin.cpp
    WGColorSelectorSettings.cpp
    WGCommonColorsCalculationRunner.cpp
    WGCommonColorSet.cpp
    WGConfig.cpp
    WGMyPaintShadeSelector.cpp
    WGQuickSettingsWidget.cpp
    WGSelectorConfigGrid.cpp
    WGSelectorPopup.cpp
    WGSelectorWidgetBase.cpp
    WGShadeLineEditor.cpp
    WGShadeSelector.cpp
    WGShadeSlider.cpp
    KisColorSourceToggle.cpp
)


ki18n_wrap_ui(KRITA_WGCOLORSELECTOR_SOURCES
    WdgQuickSettings.ui
    WdgWGSelectorSettings.ui
    WdgWGShadeLineEditor.ui
)

add_library(kritawgcolorselector MODULE ${KRITA_WGCOLORSELECTOR_SOURCES})

target_link_libraries(kritawgcolorselector kritaui)

install(TARGETS kritawgcolorselector DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  WGColorSelector.action  DESTINATION ${DATA_INSTALL_DIR}/krita/actions)
