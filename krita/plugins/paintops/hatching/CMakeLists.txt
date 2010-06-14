set(kritahatchingpaintop_PART_SRCS
    hatching_paintop_plugin.cpp
    kis_hatching_paintop.cpp
    kis_hatchingop_option.cpp
    kis_hatching_paintop_settings.cpp
    kis_hatching_paintop_settings_widget.cpp
    kis_newhatchingoptions.cpp
    hatching_brush.cpp	
    )

kde4_add_ui_files(kritahatchingpaintop_PART_SRCS newhatchingoptions.ui wdghatchingoptions.ui)

kde4_add_plugin(kritahatchingpaintop ${kritahatchingpaintop_PART_SRCS})


target_link_libraries(kritahatchingpaintop kritalibpaintop)

install(TARGETS kritahatchingpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritahatchingpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-hatching.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  3d-hatching.obj DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )
install( FILES  3d-hatching.mtl DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )


