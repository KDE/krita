

set(kritadefaultpaintops_PART_SRCS
	defaultpaintops_plugin.cc
	kis_airbrushop.cc
	kis_brushop.cc
	kis_duplicateop.cc
	kis_eraseop.cc
	kis_penop.cc
    )

kde4_automoc(${kritadefaultpaintops_PART_SRCS})

kde4_add_plugin(kritadefaultpaintops ${kritadefaultpaintops_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadefaultpaintops )

target_link_libraries(kritadefaultpaintops kritaui)

install(TARGETS kritadefaultpaintops  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadefaultpaintops.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    airbrush.png
    paintbrush.png
    eraser.png
    pencil.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


