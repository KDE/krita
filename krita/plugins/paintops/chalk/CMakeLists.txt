set(kritachalkpaintop_PART_SRCS
    chalk_paintop_plugin.cpp
    kis_chalk_paintop.cpp
    kis_chalkop_option.cpp
    kis_chalk_paintop_settings.cpp
    kis_chalk_paintop_settings_widget.cpp
    bristle.cpp
    brush_shape.cpp
    chalk_brush.cpp	
    )

kde4_add_ui_files(kritachalkpaintop_PART_SRCS wdgchalkoptions.ui )

kde4_add_plugin(kritachalkpaintop ${kritachalkpaintop_PART_SRCS})


target_link_libraries(kritachalkpaintop kritalibpaintop)

install(TARGETS kritachalkpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritachalkpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-chalk.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  3d-pencil.obj DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )
install( FILES  3d-pencil.mtl DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )


