set(kritasumipaintop_PART_SRCS
	sumipaintop_plugin.cpp
	kis_sumipaintop.cpp
	lines.cpp
    )

#kde4_add_ui_files(kritadefaultpaintops_PART_SRCS wdgbrushcurves.ui wdgduplicateop.ui )

kde4_add_plugin(kritasumipaintop ${kritasumipaintop_PART_SRCS})


target_link_libraries(kritasumipaintop kritaui)

install(TARGETS kritasumipaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasumipaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-sumi.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


