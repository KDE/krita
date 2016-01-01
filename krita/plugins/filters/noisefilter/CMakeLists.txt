set(kritanoisefilter_SOURCES
    noisefilter.cpp
    kis_wdg_noise.cpp
    )
ki18n_wrap_ui(kritanoisefilter_SOURCES
    wdgnoiseoptions.ui
    )

add_library(kritanoisefilter MODULE ${kritanoisefilter_SOURCES})
target_link_libraries(kritanoisefilter kritaui)
install(TARGETS kritanoisefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
