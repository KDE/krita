

set(kritaexample_PART_SRCS example.cc )

kde4_automoc(${kritaexample_PART_SRCS})

kde4_add_plugin(kritaexample ${kritaexample_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaexample )

target_link_libraries(kritaexample kritaui)

install(TARGETS kritaexample  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaexample.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


