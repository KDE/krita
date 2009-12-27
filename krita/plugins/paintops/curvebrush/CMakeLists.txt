set(kritacurvepaintop_PART_SRCS
    curve_brush.cpp
	curve_paintop_plugin.cpp
	kis_curve_paintop.cpp
	kis_curve_paintop_settings.cpp
    kis_curve_paintop_settings_widget.cpp
    )

kde4_add_ui_files(kritacurvepaintop_PART_SRCS wdgcurveoptions.ui )

kde4_add_plugin(kritacurvepaintop ${kritacurvepaintop_PART_SRCS})

target_link_libraries(kritacurvepaintop kritaui)

install(TARGETS kritacurvepaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacurvepaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-curve.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  stylus.obj DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )
install( FILES  stylus.mtl DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )



