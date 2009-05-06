




########### next target ###############

set(krita_xyz_u16_LIB_SRCS kis_xyz_u16_colorspace.cc )


kde4_add_library(krita_xyz_u16 SHARED ${krita_xyz_u16_LIB_SRCS})

target_link_libraries(krita_xyz_u16 pigmentcms )

set_target_properties(krita_xyz_u16 PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS krita_xyz_u16  ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(krita_xyz_u16_plugin_PART_SRCS xyz_u16_plugin.cc )


kde4_add_plugin(krita_xyz_u16_plugin ${krita_xyz_u16_plugin_PART_SRCS})



target_link_libraries(krita_xyz_u16_plugin krita_xyz_u16 pigmentcms )

install(TARGETS krita_xyz_u16_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_xyz_u16_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


