set(kritawavefilter_SOURCES
    wavefilter.cpp
    kis_wdg_wave.cpp
    )


ki18n_wrap_ui(kritawavefilter_SOURCES
    wdgwaveoptions.ui
    )

kis_add_library(kritawavefilter MODULE ${kritawavefilter_SOURCES})

target_link_libraries(kritawavefilter kritaui)

install(TARGETS kritawavefilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

