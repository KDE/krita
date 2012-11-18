set(kritaexperimentpaintop_PART_SRCS
    experiment_paintop_plugin.cpp
    kis_experiment_paintop.cpp
    kis_experimentop_option.cpp
    kis_experiment_shape_option.cpp
    kis_experiment_paintop_settings.cpp
    kis_experiment_paintop_settings_widget.cpp
    )

kde4_add_ui_files(kritaexperimentpaintop_PART_SRCS wdgexperimentoptions.ui wdgshapeoptions.ui )

kde4_add_plugin(kritaexperimentpaintop ${kritaexperimentpaintop_PART_SRCS})


target_link_libraries(kritaexperimentpaintop kritalibpaintop)

install(TARGETS kritaexperimentpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaexperimentpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-experiment.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


