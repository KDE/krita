########### next target ###############

set(koffice_graya_u16_LIB_SRCS kis_gray_u16_colorspace.cc )


kde4_add_library(koffice_graya_u16 SHARED ${koffice_graya_u16_LIB_SRCS})

target_link_libraries(koffice_graya_u16 pigmentcms)

set_target_properties(koffice_graya_u16 PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koffice_graya_u16  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(koffice_graya_u16_plugin_PART_SRCS gray_u16_plugin.cc )


kde4_add_plugin(koffice_graya_u16_plugin ${koffice_graya_u16_plugin_PART_SRCS})



target_link_libraries(koffice_graya_u16_plugin koffice_graya_u16 pigmentcms )

install(TARGETS koffice_graya_u16_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  koffice_graya_u16_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

