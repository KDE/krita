
########### next target ###############

set(kritaflakeshapes_PART_SRCS kis_image_shape.cc  kis_paint_device_shape.cc)

kde4_automoc(${kritaflakeshapes_PART_SRCS})

kde4_add_plugin(kritaflakeshapes ${kritaflakeshapes_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaflakeshapes )

target_link_libraries(kritaflakeshapes kritaui flake)

install(TARGETS kritaflakeshapes  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaflakeshapes.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
