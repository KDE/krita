

set(kritasmearybrush_PART_SRCS
    smearybrush.cc
    kis_smearyop.cc
    )

kde4_automoc(${kritasmearybrush_PART_SRCS})

kde4_add_plugin(kritasmearybrush ${kritasmearybrush_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritasmearybrush )

target_link_libraries(kritasmearybrush kritaui)

install(TARGETS kritasmearybrush  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasmearybrush.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

