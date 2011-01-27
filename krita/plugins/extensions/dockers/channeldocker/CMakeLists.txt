
set(KRITA_CHANNELDOCKER_PART_SRCS channelmodel.cpp channeldocker.cpp channeldocker_dock.cpp )

kde4_add_plugin(kritachanneldocker ${KRITA_CHANNELDOCKER_PART_SRCS})

target_link_libraries(kritachanneldocker kritaui)

install(TARGETS kritachanneldocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_channeldocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

