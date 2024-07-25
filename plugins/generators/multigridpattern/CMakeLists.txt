set(kritamultigridpatterngenerator_SOURCES
    multigridpatterngenerator.cpp
    kis_wdg_multigrid_pattern.cpp
    )


ki18n_wrap_ui(kritamultigridpatterngenerator_SOURCES
    wdgmultigridpatternoptions.ui
    )

kis_add_library(kritamultigridpatterngenerator MODULE ${kritamultigridpatterngenerator_SOURCES})

target_link_libraries(kritamultigridpatterngenerator kritaui)

install(TARGETS kritamultigridpatterngenerator  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
