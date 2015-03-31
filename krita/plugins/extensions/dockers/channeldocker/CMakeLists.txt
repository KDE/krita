
set(KRITA_CHANNELDOCKER_PART_SRCS channelmodel.cpp channeldocker.cpp channeldocker_dock.cpp )

add_library(kritachanneldocker MODULE ${KRITA_CHANNELDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritachanneldocker krita_channeldocker.desktop)

target_link_libraries(kritachanneldocker kritaui)

install(TARGETS kritachanneldocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_channeldocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

