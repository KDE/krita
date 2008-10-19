set(kritadeformpaintop_PART_SRCS
	deformpaintop_plugin.cpp
	kis_deformpaintop.cpp
	kis_deformpaintopsettings.cpp
	deform_brush.cpp	
    )

kde4_add_ui_files(kritadeformpaintop_PART_SRCS wdgdeformoptions.ui )

kde4_add_plugin(kritadeformpaintop ${kritadeformpaintop_PART_SRCS})

target_link_libraries(kritadeformpaintop kritaui)

install(TARGETS kritadeformpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadeformpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-deform.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


