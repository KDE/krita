include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

set(karbon_filtereffects_PART_SRCS
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

add_library(karbon_filtereffects MODULE ${karbon_filtereffects_PART_SRCS})

kcoreaddons_desktop_to_json(karbon_filtereffects karbon_filtereffects.desktop)

target_link_libraries(karbon_filtereffects flake kowidgets KF5::KDELibs4Support)

install(TARGETS karbon_filtereffects  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
