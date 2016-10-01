set(kritahalftonegenerator_SOURCES
    kis_halftone_generator.cpp
    )


ki18n_wrap_ui(kritahalftonegenerator_SOURCES
    wdghalftonegeneratoroptions.ui
    )

add_library(kritahalftonegenerator MODULE ${kritahalftonegenerator_SOURCES})

target_link_libraries(kritahalftonegenerator kritaui)

install(TARGETS kritahalftonegenerator  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

