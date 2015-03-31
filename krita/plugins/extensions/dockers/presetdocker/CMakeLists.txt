
set(KRITA_PRESETDOCKER_PART_SRCS presetdocker.cpp presetdocker_dock.cpp )

add_library(kritapresetdocker MODULE ${KRITA_PRESETDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritapresetdocker krita_presetdocker.desktop)

target_link_libraries(kritapresetdocker kritaui)

install(TARGETS kritapresetdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_presetdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

