add_subdirectory(tests)

set(kritacolorsmudgepaintop_SOURCES
    colorsmudge_paintop_plugin.cpp
    kis_colorsmudgeop.cpp
    kis_colorsmudgeop_settings.cpp
    kis_colorsmudgeop_settings_widget.cpp
    KisSmudgeLengthOptionData.cpp
    KisSmudgeLengthOptionModel.cpp
    KisSmudgeLengthOptionWidget.cpp
    KisSmudgeLengthOption.cpp
    KisPaintThicknessOptionData.cpp
    KisPaintThicknessOptionModel.cpp
    KisPaintThicknessOptionWidget.cpp
    KisPaintThicknessOption.cpp
    KisSmudgeOverlayModeOptionData.cpp
    KisSmudgeOverlayModeOptionModel.cpp
    KisSmudgeOverlayModeOptionWidget.cpp
    KisSmudgeRadiusOptionData.cpp
    KisGradientOption.cpp
    KisBrushPropertiesModel.cpp
    KisColorSmudgeStrategy.cpp
    KisColorSmudgeSource.cpp
    KisColorSmudgeStrategyBase.cpp
    KisColorSmudgeInterstrokeData.cpp
    KisColorSmudgeStrategyLightness.cpp
    KisColorSmudgeStrategyWithOverlay.cpp
    KisColorSmudgeStrategyMask.cpp
    KisColorSmudgeStrategyStamp.cpp
    KisColorSmudgeStrategyMaskLegacy.cpp)

kis_add_library(kritacolorsmudgepaintop MODULE ${kritacolorsmudgepaintop_SOURCES})

target_link_libraries(kritacolorsmudgepaintop kritalibpaintop)

install(TARGETS kritacolorsmudgepaintop DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-colorsmudge.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

