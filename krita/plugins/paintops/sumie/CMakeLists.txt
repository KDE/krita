set(kritasumipaintop_PART_SRCS
    sumi_paintop_plugin.cpp
    kis_sumi_paintop.cpp
    kis_sumi_shape_option.cpp
    kis_sumi_ink_option.cpp
    kis_sumi_paintop_settings.cpp
    kis_sumi_paintop_settings_widget.cpp
    bristle.cpp
    brush_shape.cpp
    brush.cpp
    trajectory.cpp
    )

kde4_add_ui_files(kritasumipaintop_PART_SRCS wdgInkOptions.ui  wdgshapeoptions.ui )

kde4_add_plugin(kritasumipaintop ${kritasumipaintop_PART_SRCS})

target_link_libraries(kritasumipaintop kritalibpaintop)

install(TARGETS kritasumipaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasumipaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-sumi.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


