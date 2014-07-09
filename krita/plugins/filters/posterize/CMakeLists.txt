

set(kritaposterize_PART_SRCS posterize.cpp )

kde4_add_plugin(kritaposterize ${kritaposterize_PART_SRCS})



target_link_libraries(kritaposterize kritaui)

install(TARGETS kritaposterize  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaposterize.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


