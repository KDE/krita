set(KRITA_OVERVIEWDOCKER_SOURCES overviewdocker.cpp overviewdocker_dock.cpp overviewwidget.cc )
kis_add_library(kritaoverviewdocker MODULE ${KRITA_OVERVIEWDOCKER_SOURCES})
target_link_libraries(kritaoverviewdocker kritaui)
install(TARGETS kritaoverviewdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
