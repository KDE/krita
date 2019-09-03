set(kritarandompickfilter_SOURCES
    randompickfilter.cpp
    kis_wdg_random_pick.cpp
    )


ki18n_wrap_ui(kritarandompickfilter_SOURCES
    wdgrandompickoptions.ui
    )

kis_add_library(kritarandompickfilter MODULE ${kritarandompickfilter_SOURCES})

target_link_libraries(kritarandompickfilter kritaui)

install(TARGETS kritarandompickfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
