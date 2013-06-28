
set(KRITA_PALETTEDOCKER_PART_SRCS colorsetchooser.cpp palettemodel.cpp palettedocker.cpp palettedocker_dock.cpp )

kde4_add_ui_files(KRITA_PALETTEDOCKER_PART_SRCS
    wdgpalettedock.ui
)

kde4_add_plugin(kritapalettedocker ${KRITA_PALETTEDOCKER_PART_SRCS})

target_link_libraries(kritapalettedocker kritaui)

install(TARGETS kritapalettedocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_palettedocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

