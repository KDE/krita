set(krita_gray_u16_plugin_PART_SRCS gray_u16_plugin.cc kis_gray_u16_colorspace.cc )

kde4_add_plugin(krita_gray_u16_plugin ${krita_gray_u16_plugin_PART_SRCS})



target_link_libraries(krita_gray_u16_plugin pigmentcms )

install(TARGETS krita_gray_u16_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_gray_u16_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
