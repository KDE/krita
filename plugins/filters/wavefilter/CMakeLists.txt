set(kritawavefilter_SOURCES
    wavefilter.cpp
    kis_wdg_wave.cpp
    )


ki18n_wrap_ui(kritawavefilter_SOURCES
    wdgwaveoptions.ui
    )

add_library(kritawavefilter MODULE ${kritawavefilter_SOURCES})

target_link_libraries(kritawavefilter kritaui)

install(TARGETS kritawavefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

