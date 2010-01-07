# Painterly Complex Paintop

set(kritacomplexbrush_PART_SRCS
    complexbrush.cpp
    kis_complexop.cpp
    kis_complexop_settings.cpp
    kis_complexop_settings_widget.cpp
    )

kde4_add_ui_files(kritacomplexbrush_PART_SRCS wdgbrushcurves.ui)
kde4_add_plugin(kritacomplexbrush ${kritacomplexbrush_PART_SRCS})
target_link_libraries(kritacomplexbrush pigmentcms kritalibpaintop)
install(TARGETS kritacomplexbrush DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritacomplexbrush.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES krita-mixing.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

