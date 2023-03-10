set(kritacurvepaintop_SOURCES
      curve_paintop_plugin.cpp
      kis_curve_paintop.cpp
      kis_curve_paintop_settings.cpp
      kis_curve_paintop_settings_widget.cpp
      KisCurveOpOptionData.cpp
      KisCurveOpOptionModel.cpp
      KisCurveOpOptionWidget.cpp
      
    )

ki18n_wrap_ui(kritacurvepaintop_SOURCES wdgcurveoptions.ui )

kis_add_library(kritacurvepaintop MODULE ${kritacurvepaintop_SOURCES})

target_link_libraries(kritacurvepaintop kritalibpaintop)

install(TARGETS kritacurvepaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-curve.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
