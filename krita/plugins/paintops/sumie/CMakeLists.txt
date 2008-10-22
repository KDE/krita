set(kritasumi_paintop_PART_SRCS
	sumi_paintop_plugin.cpp
	kis_sumi_paintop.cpp
	kis_sumi_paintop_settings.cpp
	stroke_sample.cpp
	bristle.cpp
	brush_shape.cpp
	brush.cpp	
	gauss.cpp
	trajectory.cpp
    )

kde4_add_ui_files(kritasumi_paintop_PART_SRCS wdgsumieoptions.ui )

kde4_add_plugin(kritasumi_paintop ${kritasumi_paintop_PART_SRCS})

target_link_libraries(kritasumi_paintop kritaui)

install(TARGETS kritasumi_paintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasumi_paintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita-sumi.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


