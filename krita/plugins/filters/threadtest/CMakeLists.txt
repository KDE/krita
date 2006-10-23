

set(kritathreadtest_PART_SRCS threadtest.cc )

kde4_automoc(${kritathreadtest_PART_SRCS})

kde4_add_plugin(kritathreadtest ${kritathreadtest_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritathreadtest )

target_link_libraries(kritathreadtest kritaui)

install(TARGETS kritathreadtest  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritathreadtest.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


