set(kritahairypaintop_SOURCES
    hairy_paintop_plugin.cpp
    kis_hairy_paintop.cpp
    kis_hairy_paintop_settings.cpp
    kis_hairy_paintop_settings_widget.cpp
    bristle.cpp
    hairy_brush.cpp
    trajectory.cpp
    KisHairyBristleOptionData.cpp
    KisHairyBristleOptionModel.cpp
    KisHairyBristleOptionWidget.cpp
    KisHairyInkOptionData.cpp
    KisHairyInkOptionModel.cpp
    KisHairyInkOptionWidget.cpp
    )

ki18n_wrap_ui(kritahairypaintop_SOURCES wdgInkOptions.ui  wdghairyshapeoptions.ui wdgbristleoptions.ui)

kis_add_library(kritahairypaintop MODULE ${kritahairypaintop_SOURCES})

target_link_libraries(kritahairypaintop kritalibpaintop)

install(TARGETS kritahairypaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-sumi.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


