
set(KRITA_COMPOSITIONDOCKER_PART_SRCS compositionmodel.cpp compositiondocker.cpp compositiondocker_dock.cpp )

kde4_add_ui_files(KRITA_COMPOSITIONDOCKER_PART_SRCS
    wdgcompositiondocker.ui
)
kde4_add_plugin(kritacompositiondocker ${KRITA_COMPOSITIONDOCKER_PART_SRCS})

target_link_libraries(kritacompositiondocker kritaui)

install(TARGETS kritacompositiondocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_compositiondocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

