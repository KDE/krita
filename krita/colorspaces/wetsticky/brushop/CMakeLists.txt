

set(kritawsbrushpaintop_PART_SRCS
    wsbrushpaintop_plugin.cc
    kis_wsbrushop.cc
    )

kde4_automoc(${kritawsbrushpaintop_PART_SRCS})

kde4_add_plugin(kritawsbrushpaintop ${kritawsbrushpaintop_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritawsbrushpaintop )

target_link_libraries(kritawsbrushpaintop kritaui)

install(TARGETS kritawsbrushpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritawsbrushpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES wetpaintbrush.png  DESTINATION ${DATA_INSTALL_DIR}/apps/krita/images)


