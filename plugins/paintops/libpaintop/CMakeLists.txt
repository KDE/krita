set(kritalibpaintop_LIB_SRCS
    kis_auto_brush_widget.cpp
    kis_brush_based_paintop.cpp
    kis_predefined_brush_chooser.cpp
    KisAutoBrushModel.cpp
    KisPredefinedBrushModel.cpp
    kis_brush_option_widget.cpp
    kis_brush_option.cpp
    kis_brush_selection_widget.cpp
    KisColorOptionData.cpp
    KisColorOptionModel.cpp
    KisColorOptionWidget.cpp
    kis_color_source.cpp
    kis_color_source_option.cpp
    KisCurveOptionDataUniformProperty.cpp
    KisSensorData.cpp
    KisSensorPackInterface.cpp
    KisKritaSensorPack.cpp
    KisCurveOptionDataCommon.cpp
    KisCurveOptionData.cpp
    KisCurveOptionInputControlsStrategyInterface.cpp
    KisCurveOptionInputControlsStrategy.cpp
    KisCurveOptionRangeControlsStrategyInterface.cpp
    KisCurveRangeModelInterface.cpp
    KisCurveRangeModel.cpp
    KisCurveOptionModel.cpp
    KisCurveOptionWidget.cpp
    KisCurveOption.cpp
    KisMultiSensorsSelector.cpp
    KisMultiSensorsModel.cpp
    KisDynamicSensorFactory.cpp
    KisDynamicSensorFactoryRegistry.cpp
    KisSimpleDynamicSensorFactory.cpp
    KisSpacingOptionWidget.cpp
    KisSpacingOptionData.cpp
    KisSpacingOptionModel.cpp
    KisSpacingOption.cpp
    KisMirrorOptionData.cpp
    KisMirrorOptionModel.cpp
    KisMirrorOptionWidget.cpp
    KisMirrorOption.cpp
    KisSharpnessOptionData.cpp
    KisSharpnessOptionModel.cpp
    KisSharpnessOptionWidget.cpp
    KisSharpnessOption.cpp
    KisScatterOptionData.cpp
    KisScatterOptionModel.cpp
    KisScatterOptionWidget.cpp
    KisScatterOption.cpp
    KisAirbrushOptionData.cpp
    KisAirbrushOptionModel.cpp
    KisAirbrushOptionWidget.cpp
    KisCompositeOpOptionData.cpp
    KisCompositeOpOptionModel.cpp
    KisCompositeOpOptionWidget.cpp
    KisPaintingModeOptionData.cpp
    KisPaintingModeOptionModel.cpp
    KisPaintingModeOptionWidget.cpp
    KisColorSourceOptionData.cpp
    KisColorSourceOptionModel.cpp
    KisColorSourceOptionWidget.cpp
    KisLightnessStrengthOptionWidget.cpp
    KisTextureOptionData.cpp
    KisEmbeddedTextureData.cpp
    KisTextureOptionModel.cpp
    KisTextureOptionWidget.cpp
    KisSizeOptionData.cpp
    KisSizeOptionWidget.cpp
    KisStandardOptionData.cpp
    KisStandardOptions.cpp
    KisRotationOption.cpp
    KisOpacityOption.cpp
    KisFlowOpacityOption.cpp
    KisDarkenOption.cpp
    KisHSVOption.cpp
    KisFilterOptionData.cpp
    KisFilterOptionModel.cpp
    KisFilterOptionWidget.cpp
    kis_custom_brush_widget.cpp
    kis_clipboard_brush_widget.cpp
    KisDabCacheUtils.cpp
    kis_dab_cache_base.cpp
    kis_dab_cache.cpp
    kis_precision_option.cpp
    kis_current_outline_fetcher.cpp
    kis_text_brush_chooser.cpp
    kis_brush_based_paintop_options_widget.cpp
    kis_brush_based_paintop_settings.cpp
    kis_texture_option.cpp
    kis_texture_chooser.cpp
    KisTextureMaskInfo.cpp
    KisMaskingBrushOption.cpp
    KisMaskingBrushOptionProperties.cpp
    sensors/KisDynamicSensor.cpp
    sensors/KisDynamicSensors.cpp
    sensors/KisDynamicSensorDrawingAngle.cpp
    sensors/KisDynamicSensorFuzzy.cpp
    sensors/KisDynamicSensorDistance.cpp
    sensors/KisDynamicSensorFade.cpp
    sensors/KisDynamicSensorTime.cpp
    KisDynamicSensorFactoryTime.cpp
    KisDynamicSensorFactoryFade.cpp
    KisDynamicSensorFactoryDistance.cpp
    KisDynamicSensorFactoryDrawingAngle.cpp
    KisSensorWithLengthModel.cpp
    KisDrawingAngleSensorModel.cpp
    KisTextBrushModel.cpp
)

ki18n_wrap_ui(kritalibpaintop_LIB_SRCS
    forms/wdgautobrush.ui
    forms/wdgBrushSizeOptions.ui
    forms/wdgcurveoption.ui
    forms/wdgcurveoption2.ui
    forms/wdgcustombrush.ui
    forms/wdgclipboardbrush.ui
    forms/wdgtextbrush.ui
    forms/wdgincremental.ui
    forms/wdgmultisensorsselector.ui
    forms/wdgairbrush.ui
    forms/wdgfilteroption.ui
    forms/wdgcoloroptions.ui
    forms/wdgbrushchooser.ui
    forms/wdgpredefinedbrushchooser.ui
    forms/wdgtexturechooser.ui
    forms/wdgCompositeOpOption.ui
    forms/wdgflowopacityoption.ui
    sensors/SensorDistanceConfiguration.ui
    sensors/SensorTimeConfiguration.ui
    sensors/SensorFadeConfiguration.ui

)

kis_add_library(kritalibpaintop SHARED ${kritalibpaintop_LIB_SRCS} )
generate_export_header(kritalibpaintop BASE_NAME kritapaintop EXPORT_MACRO_NAME PAINTOP_EXPORT)

target_link_libraries(kritalibpaintop kritaui kritalibbrush kritawidgetutils)
target_link_libraries(kritalibpaintop LINK_INTERFACE_LIBRARIES kritaui kritalibbrush)

set_target_properties(kritalibpaintop PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibpaintop  ${INSTALL_TARGETS_DEFAULT_ARGS})


add_subdirectory(tests)
