set(kritacurvepaintop_PART_SRCS
      curve_brush.cpp
      curve_paintop_plugin.cpp
      kis_curve_paintop.cpp
      kis_curve_paintop_settings.cpp
      kis_curve_line_option.cpp
      kis_linewidth_option.cpp
      kis_curves_opacity_option.cpp
      kis_curve_paintop_settings_widget.cpp
    )

ki18n_wrap_ui(kritacurvepaintop_PART_SRCS wdgcurveoptions.ui )

add_library(kritacurvepaintop MODULE ${kritacurvepaintop_PART_SRCS})
kcoreaddons_desktop_to_json(kritacurvepaintop kritacurvepaintop.desktop)

target_link_libraries(kritacurvepaintop kritalibpaintop)

install(TARGETS kritacurvepaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacurvepaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  krita-curve.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
