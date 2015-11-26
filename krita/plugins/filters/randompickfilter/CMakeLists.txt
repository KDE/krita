set(kritarandompickfilter_SOURCES
    randompickfilter.cpp
    kis_wdg_random_pick.cpp
    )


ki18n_wrap_ui(kritarandompickfilter_SOURCES
    wdgrandompickoptions.ui
    )

add_library(kritarandompickfilter MODULE ${kritarandompickfilter_SOURCES})
kcoreaddons_desktop_to_json(kritarandompickfilter kritarandompickfilter.desktop)

target_link_libraries(kritarandompickfilter kritaui)

install(TARGETS kritarandompickfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
