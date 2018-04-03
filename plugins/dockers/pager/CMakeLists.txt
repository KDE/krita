set(KRITA_PAGERDOCKER_SOURCES PagerDockerPlugin.cpp PagerDockerDock.cpp)
add_library(kritapagerdocker MODULE ${KRITA_PAGERDOCKER_SOURCES})
target_link_libraries(kritapagerdocker kritaui)
install(TARGETS kritapagerdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
