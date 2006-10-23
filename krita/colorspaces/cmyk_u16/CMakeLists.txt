




########### next target ###############

set(krita_cmyk_u16_LIB_SRCS kis_cmyk_u16_colorspace.cc )

kde4_automoc(${krita_cmyk_u16_LIB_SRCS})

kde4_add_library(krita_cmyk_u16 SHARED ${krita_cmyk_u16_LIB_SRCS})

target_link_libraries(krita_cmyk_u16  ${KDE4_KDECORE_LIBS} kritacolor )

set_target_properties(krita_cmyk_u16 PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krita_cmyk_u16  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############

set(krita_cmyk_u16_plugin_PART_SRCS cmyk_u16_plugin.cc )

kde4_automoc(${krita_cmyk_u16_plugin_PART_SRCS})

kde4_add_plugin(krita_cmyk_u16_plugin ${krita_cmyk_u16_plugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krita_cmyk_u16_plugin )

target_link_libraries(krita_cmyk_u16_plugin  ${KDE4_KDECORE_LIBS} krita_cmyk_u16 kritacolor )

install(TARGETS krita_cmyk_u16_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_cmyk_u16_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


