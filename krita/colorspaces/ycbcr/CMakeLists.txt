

########### next target ###############

set(kritaycbcrplugin_PART_SRCS ycbcr_plugin.cc kis_ycbcr_u16_colorspace.cc kis_ycbcr_u8_colorspace.cc )

kde4_automoc(${kritaycbcrplugin_PART_SRCS})

kde4_add_plugin(kritaycbcrplugin ${kritaycbcrplugin_PART_SRCS})



target_link_libraries(kritaycbcrplugin  ${KDE4_KDECORE_LIBS} pigment)

install(TARGETS kritaycbcrplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaycbcrplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
