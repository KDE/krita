set(kritaspraypaintop_PART_SRCS
    spray_paintop_plugin.cpp
    kis_spray_paintop.cpp
    kis_sprayop_option.cpp
    kis_spray_shape_option.cpp
    kis_spray_paintop_settings.cpp
    kis_spray_paintop_settings_widget.cpp
    spray_brush.cpp
    random_gauss.cpp
    )

kde4_add_ui_files(kritaspraypaintop_PART_SRCS wdgsprayoptions.ui wdgshapeoptions.ui )

kde4_add_plugin(kritaspraypaintop ${kritaspraypaintop_PART_SRCS})


target_link_libraries(kritaspraypaintop kritalibpaintop)

install(TARGETS kritaspraypaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaspraypaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-spray.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


