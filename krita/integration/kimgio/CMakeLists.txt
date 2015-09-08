include_directories( ${CMAKE_SOURCE_DIR}/krita/ui/ora ${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kimg_kra_LIB_SRCS kra.cpp)
add_library(kimg_kra MODULE ${kimg_kra_LIB_SRCS})
target_link_libraries(kimg_kra Qt5::Gui Kf5::Archive)

set(kimg_ora_LIB_SRCS ora.cpp)
add_library(kimg_ora MODULE ${kimg_ora_LIB_SRCS})
target_link_libraries(kimg_ora Qt5::Gui Kf5::Archive)

install(TARGETS kimg_ora DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )
install(TARGETS kimg_kra DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/imageformats/ )

install( FILES
  ora.desktop
  kra.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/qimageioplugins/ )
