set(KRITA_PRESETDOCKER_SOURCES presetdocker.cpp presetdocker_dock.cpp )
kis_add_library(kritapresetdocker MODULE ${KRITA_PRESETDOCKER_SOURCES})
target_link_libraries(kritapresetdocker kritaui)
install(TARGETS kritapresetdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
