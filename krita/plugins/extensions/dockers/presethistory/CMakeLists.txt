
set(KRITA_PRESETHISTORY_PART_SRCS presethistory.cpp presethistory_dock.cpp )

kde4_add_plugin(kritapresethistory ${KRITA_PRESETHISTORY_PART_SRCS})

target_link_libraries(kritapresethistory kritaui)

install(TARGETS kritapresethistory  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_presethistory.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

