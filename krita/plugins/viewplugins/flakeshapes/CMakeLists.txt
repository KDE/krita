
########### next target ###############

set(kritaflakeshapes_PART_SRCS kis_image_shape.cc  kis_paint_device_shape.cc)

kde4_automoc(${kritaflakeshapes_PART_SRCS})

kde4_add_ui3_files(kritaflakeshapes_PART_SRCS )

kde4_add_plugin(kritaflakeshapes ${kritaflakeshapes_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaflakeshapes )

target_link_libraries(kritaflakeshapes kritaui)

install_targets(${PLUGIN_INSTALL_DIR} kritaflakeshapes )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES  kritaflakeshapes.desktop )
