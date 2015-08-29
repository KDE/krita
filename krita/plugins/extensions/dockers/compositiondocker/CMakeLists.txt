
set(KRITA_COMPOSITIONDOCKER_PART_SRCS compositionmodel.cpp compositiondocker.cpp compositiondocker_dock.cpp )

ki18n_wrap_ui(KRITA_COMPOSITIONDOCKER_PART_SRCS
    wdgcompositiondocker.ui
)
add_library(kritacompositiondocker MODULE ${KRITA_COMPOSITIONDOCKER_PART_SRCS})
kcoreaddons_desktop_to_json(kritacompositiondocker krita_compositiondocker.desktop)

target_link_libraries(kritacompositiondocker ${KDE4_KFILE_LIBRARY} kritaui)

install(TARGETS kritacompositiondocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_compositiondocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

