set(kritahatchingpaintop_PART_SRCS
    hatching_paintop_plugin.cpp
    kis_hatching_paintop.cpp
    kis_hatching_options.cpp
    kis_hatching_preferences.cpp
    kis_hatching_paintop_settings.cpp
    kis_hatching_paintop_settings_widget.cpp
    kis_hatching_pressure_crosshatching_option.cpp
    kis_hatching_pressure_separation_option.cpp
    kis_hatching_pressure_thickness_option.cpp
    hatching_brush.cpp
    )

kde4_add_ui_files(kritahatchingpaintop_PART_SRCS wdghatchingpreferences.ui wdghatchingoptions.ui)

kde4_add_plugin(kritahatchingpaintop ${kritahatchingpaintop_PART_SRCS})


target_link_libraries(kritahatchingpaintop kritalibpaintop)

install(TARGETS kritahatchingpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritahatchingpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-hatching.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

