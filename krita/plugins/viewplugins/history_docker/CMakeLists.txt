

set(kritahistorydocker_PART_SRCS historydocker.cc )

kde4_automoc(${kritahistorydocker_PART_SRCS})

kde4_add_plugin(kritahistorydocker ${kritahistorydocker_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistorydocker )

target_link_libraries(kritahistorydocker kritaui)

install(TARGETS kritahistorydocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

