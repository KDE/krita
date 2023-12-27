kis_add_library(kritasamplescreencolor MODULE KisSampleScreenColor.cpp)
target_link_libraries(kritasamplescreencolor kritaui)
install(TARGETS kritasamplescreencolor  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  samplescreencolor.xmlgui  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
