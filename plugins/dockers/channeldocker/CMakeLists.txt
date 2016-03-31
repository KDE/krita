set(KRITA_CHANNELDOCKER_SOURCES channelmodel.cpp channeldocker.cpp channeldocker_dock.cpp )
add_library(kritachanneldocker MODULE ${KRITA_CHANNELDOCKER_SOURCES})
target_link_libraries(kritachanneldocker kritaui)
install(TARGETS kritachanneldocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
