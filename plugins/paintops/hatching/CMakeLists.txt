set(kritahatchingpaintop_SOURCES
    hatching_paintop_plugin.cpp
    kis_hatching_paintop.cpp
    kis_hatching_options.cpp
    kis_hatching_preferences.cpp
    kis_hatching_paintop_settings.cpp
    kis_hatching_paintop_settings_widget.cpp
    kis_hatching_pressure_angle_option.cpp
    kis_hatching_pressure_crosshatching_option.cpp
    kis_hatching_pressure_separation_option.cpp
    kis_hatching_pressure_thickness_option.cpp
    hatching_brush.cpp
    )

ki18n_wrap_ui(kritahatchingpaintop_SOURCES wdghatchingpreferences.ui wdghatchingoptions.ui)

kis_add_library(kritahatchingpaintop MODULE ${kritahatchingpaintop_SOURCES})

target_link_libraries(kritahatchingpaintop kritalibpaintop)

install(TARGETS kritahatchingpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-hatching.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

