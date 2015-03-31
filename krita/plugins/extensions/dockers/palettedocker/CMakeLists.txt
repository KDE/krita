
set(KRITA_PALETTEDOCKER_PART_SRCS colorsetchooser.cpp palettemodel.cpp palettedocker.cpp palettedocker_dock.cpp )

ki18n_wrap_ui(KRITA_PALETTEDOCKER_PART_SRCS
    wdgpalettedock.ui
)

add_library(kritapalettedocker MODULE ${KRITA_PALETTEDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritapalettedocker krita_palettedocker.desktop)

target_link_libraries(kritapalettedocker kritaui)

install(TARGETS kritapalettedocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_palettedocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

