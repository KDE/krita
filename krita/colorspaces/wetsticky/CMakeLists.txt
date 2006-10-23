

add_subdirectory( brushop )

########### next target ###############

set(kritawsplugin_PART_SRCS wet_sticky_plugin.cc kis_wet_sticky_colorspace.cc kis_ws_engine_filter.cc )

kde4_automoc(${kritawsplugin_PART_SRCS})

kde4_add_plugin(kritawsplugin ${kritawsplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritawsplugin )

target_link_libraries(kritawsplugin  ${KDE4_KDECORE_LIBS} kritacolor )

install(TARGETS kritawsplugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritawsplugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

