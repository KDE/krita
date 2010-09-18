
set(KRITA_PRESETDOCKER_PART_SRCS presetdocker.cpp presetdocker_dock.cpp )

kde4_add_plugin(kritapresetdocker ${KRITA_PRESETDOCKER_PART_SRCS})

target_link_libraries(kritapresetdocker kritaui)

install(TARGETS kritapresetdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_presetdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

