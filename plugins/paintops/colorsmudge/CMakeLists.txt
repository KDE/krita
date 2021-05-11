add_subdirectory(tests)

set(kritacolorsmudgepaintop_SOURCES
    colorsmudge_paintop_plugin.cpp
    kis_colorsmudgeop.cpp
    kis_colorsmudgeop_settings.cpp
    kis_colorsmudgeop_settings_widget.cpp
    kis_rate_option.cpp
    kis_smudge_option.cpp
    kis_smudge_option_widget.cpp
    kis_smudge_radius_option.cpp
    KisColorSmudgeStrategy.cpp
    KisColorSmudgeSource.cpp
    KisColorSmudgeStrategyBase.cpp
    KisColorSmudgeInterstrokeData.cpp
    KisColorSmudgeStrategyLightness.cpp
    KisColorSmudgeStrategySmearLightness.cpp
    KisColorSmudgeStrategyWithOverlay.cpp
    KisColorSmudgeStrategyMask.cpp
    KisColorSmudgeStrategyStamp.cpp
    KisColorSmudgeStrategyMaskLegacy.cpp)

add_library(kritacolorsmudgepaintop MODULE ${kritacolorsmudgepaintop_SOURCES})

target_link_libraries(kritacolorsmudgepaintop kritalibpaintop)

install(TARGETS kritacolorsmudgepaintop DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-colorsmudge.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

