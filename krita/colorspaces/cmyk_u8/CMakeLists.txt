

add_subdirectory( templates )




########### next target ###############

set(kritacmykplugin_PART_SRCS cmyk_plugin.cc kis_cmyk_colorspace.cc )

kde4_automoc(${kritacmykplugin_PART_SRCS})

kde4_add_plugin(kritacmykplugin ${kritacmykplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacmykplugin )

target_link_libraries(kritacmykplugin  ${KDE4_KDECORE_LIBS} kritacolor )

install(TARGETS kritacmykplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacmykplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

