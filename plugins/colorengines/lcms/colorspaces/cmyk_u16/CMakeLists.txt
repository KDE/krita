########### next target ###############

set(krita_cmyk_u16_plugin_PART_SRCS cmyk_u16_plugin.cc kis_cmyk_u16_colorspace.cc )

kde4_add_plugin(krita_cmyk_u16_plugin ${krita_cmyk_u16_plugin_PART_SRCS})



target_link_libraries(krita_cmyk_u16_plugin pigmentcms)

install(TARGETS krita_cmyk_u16_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_cmyk_u16_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
