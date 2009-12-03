

set(kritadodgeburn_PART_SRCS DodgeBurnPlugin.cpp Dodge.cpp )

kde4_add_plugin(kritadodgeburn ${kritadodgeburn_PART_SRCS})



target_link_libraries(kritadodgeburn kritaui)

install(TARGETS kritadodgeburn  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritadodgeburn.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



