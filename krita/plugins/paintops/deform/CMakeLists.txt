set(kritadeformpaintop_PART_SRCS
    deform_brush.cpp
    deform_paintop_plugin.cpp
    kis_deform_paintop.cpp
    kis_deform_paintop_settings.cpp
    kis_deform_paintop_factory.cpp
    kis_deform_option.cpp
    kis_deform_paintop_settings_widget.cpp
    )

kde4_add_ui_files(kritadeformpaintop_PART_SRCS wdgdeformoptions.ui )

kde4_add_plugin(kritadeformpaintop ${kritadeformpaintop_PART_SRCS})

target_link_libraries(kritadeformpaintop kritaui kritalibpaintop)

install(TARGETS kritadeformpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadeformpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-deform.png DESTINATION ${DATA_INSTALL_DIR}/krita/images )
install( FILES  3d-deform-brush.obj DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )
install( FILES  3d-deform-brush.mtl DESTINATION ${DATA_INSTALL_DIR}/krita/brushmodels )
