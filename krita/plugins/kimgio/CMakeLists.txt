include_directories( ${CMAKE_SOURCE_DIR}/krita/ui/ora ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kimg_kra_LIB_SRCS kra.cpp)
kde4_add_plugin(kimg_kra ${kimg_kra_LIB_SRCS})
target_link_libraries(kimg_kra kritaui)

set(kimg_ora_LIB_SRCS ora.cpp)
kde4_add_plugin(kimg_ora ${kimg_ora_LIB_SRCS})
target_link_libraries(kimg_ora kritaui)

install(TARGETS kimg_ora DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
install(TARGETS kimg_kra DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

install( FILES
  ora.desktop
  kra.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/qimageioplugins/ )
