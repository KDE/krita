set(kritachalkpaintop_PART_SRCS
	chalkpaintop_plugin.cpp
	kis_chalkpaintop.cpp
	bristle.cpp
	brush_shape.cpp
	brush.cpp	
    )

#kde4_add_ui_files(kritadefaultpaintops_PART_SRCS wdgbrushcurves.ui wdgduplicateop.ui )

kde4_add_plugin(kritachalkpaintop ${kritachalkpaintop_PART_SRCS})


target_link_libraries(kritachalkpaintop kritaui)

install(TARGETS kritachalkpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritachalkpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-chalk.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


