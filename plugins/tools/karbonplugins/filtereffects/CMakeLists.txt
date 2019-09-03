set(karbon_filtereffects_SOURCES
    FilterEffectsPlugin.cpp
    BlurEffect.cpp
    BlurEffectFactory.cpp
    BlurEffectConfigWidget.cpp
    OffsetEffect.cpp
    OffsetEffectFactory.cpp
    OffsetEffectConfigWidget.cpp
    MergeEffect.cpp
    MergeEffectFactory.cpp
    MergeEffectConfigWidget.cpp
    ColorMatrixEffect.cpp
    ColorMatrixEffectFactory.cpp
    ColorMatrixEffectConfigWidget.cpp
    FloodEffect.cpp
    FloodEffectFactory.cpp
    FloodEffectConfigWidget.cpp
    CompositeEffect.cpp
    CompositeEffectFactory.cpp
    CompositeEffectConfigWidget.cpp
    BlendEffect.cpp
    BlendEffectFactory.cpp
    BlendEffectConfigWidget.cpp
    ComponentTransferEffect.cpp
    ComponentTransferEffectFactory.cpp
    ComponentTransferEffectConfigWidget.cpp
    ImageEffect.cpp
    ImageEffectFactory.cpp
    ImageEffectConfigWidget.cpp
    MorphologyEffect.cpp
    MorphologyEffectFactory.cpp
    MorphologyEffectConfigWidget.cpp
    ConvolveMatrixEffect.cpp
    ConvolveMatrixEffectFactory.cpp
    ConvolveMatrixEffectConfigWidget.cpp
    MatrixDataModel.cpp
    )

kis_add_library(krita_filtereffects MODULE ${karbon_filtereffects_SOURCES})

target_link_libraries(krita_filtereffects kritaflake kritawidgets kritaui KF5::Completion)

install(TARGETS krita_filtereffects  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
