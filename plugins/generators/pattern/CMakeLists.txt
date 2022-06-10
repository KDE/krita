set(kritapatterngenerator_SOURCES
    patterngenerator.cpp
    kis_wdg_pattern.cpp
    )


ki18n_wrap_ui(kritapatterngenerator_SOURCES
    wdgpatternoptions.ui
    )

kis_add_library(kritapatterngenerator MODULE ${kritapatterngenerator_SOURCES})

target_link_libraries(kritapatterngenerator kritaui)

install(TARGETS kritapatterngenerator  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
