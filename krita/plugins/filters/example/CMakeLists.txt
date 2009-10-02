

set(kritaexample_PART_SRCS example.cpp )

kde4_add_plugin(kritaexample ${kritaexample_PART_SRCS})



target_link_libraries(kritaexample kritaui)

install(TARGETS kritaexample  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaexample.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


