set(kritadynapaintop_PART_SRCS
    dyna_paintop_plugin.cpp
    kis_dyna_paintop.cpp
    kis_dynaop_option.cpp
    kis_dyna_paintop_settings.cpp
    kis_dyna_paintop_settings_widget.cpp
    dyna_brush.cpp
    filter.cpp
    )

kde4_add_ui_files(kritadynapaintop_PART_SRCS wdgdynaoptions.ui )

kde4_add_plugin(kritadynapaintop ${kritadynapaintop_PART_SRCS})


target_link_libraries(kritadynapaintop kritalibpaintop)

install(TARGETS kritadynapaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadynapaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-dyna.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


