set(kritahairypaintop_PART_SRCS
    hairy_paintop_plugin.cpp
    kis_hairy_paintop.cpp
    kis_hairy_bristle_option.cpp
    kis_hairy_shape_option.cpp
    kis_hairy_ink_option.cpp
    kis_hairy_paintop_settings.cpp
    kis_hairy_paintop_settings_widget.cpp
    bristle.cpp
    hairy_brush.cpp
    trajectory.cpp
    )

kde4_add_ui_files(kritahairypaintop_PART_SRCS wdgInkOptions.ui  wdgshapeoptions.ui wdgbristleoptions.ui)

kde4_add_plugin(kritahairypaintop ${kritahairypaintop_PART_SRCS})

target_link_libraries(kritahairypaintop kritalibpaintop)

install(TARGETS kritahairypaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritahairypaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-sumi.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


