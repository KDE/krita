set(KRITA_PRESETDOCKER_SOURCES presetdocker.cpp presetdocker_dock.cpp )
add_library(kritapresetdocker MODULE ${KRITA_PRESETDOCKER_SOURCES})
target_link_libraries(kritapresetdocker kritaui)
install(TARGETS kritapresetdocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
