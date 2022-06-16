set(kritasimplexnoisegenerator_SOURCES
    simplexnoisegenerator.cpp
    kis_wdg_simplex_noise.cpp
    3rdparty/c-open-simplex/open-simplex-noise.c
    )
ki18n_wrap_ui(kritasimplexnoisegenerator_SOURCES
    wdgsimplexnoiseoptions.ui
    )

add_library(kritasimplexnoisegenerator MODULE ${kritasimplexnoisegenerator_SOURCES})
target_link_libraries(kritasimplexnoisegenerator kritaui)
install(TARGETS kritasimplexnoisegenerator  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
