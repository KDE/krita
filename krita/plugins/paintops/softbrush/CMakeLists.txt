set(kritasoftpaintop_PART_SRCS
    soft_paintop_plugin.cpp
    kis_soft_paintop.cpp
    kis_softop_option.cpp
    kis_soft_paintop_settings.cpp
    kis_soft_paintop_settings_widget.cpp
    kis_alpha_mask.cpp
    )

kde4_add_ui_files(kritasoftpaintop_PART_SRCS wdgsoftoptions.ui )

kde4_add_plugin(kritasoftpaintop ${kritasoftpaintop_PART_SRCS})


target_link_libraries(kritasoftpaintop kritalibpaintop)

install(TARGETS kritasoftpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasoftpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES
#    krita-softbrush.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


