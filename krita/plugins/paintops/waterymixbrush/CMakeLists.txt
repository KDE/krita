set(kritawaterymixpaintop_PART_SRCS
    waterymix_paintop_plugin.cpp
    kis_waterymix_paintop.cpp
    kis_waterymix_options.cpp
    kis_waterymix_paintop_settings.cpp
    kis_waterymix_paintop_settings_widget.cpp
    )

kde4_add_ui_files(kritawaterymixpaintop_PART_SRCS wdgwaterymixoptions.ui)

kde4_add_plugin(kritawaterymixpaintop ${kritawaterymixpaintop_PART_SRCS})


target_link_libraries(kritawaterymixpaintop kritalibpaintop)

install(TARGETS kritawaterymixpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritawaterymixpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-waterymix.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

