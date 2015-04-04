

set(kritarandompickfilter_PART_SRCS
    randompickfilter.cpp
    kis_wdg_random_pick.cpp
    )


ki18n_wrap_ui(kritarandompickfilter_PART_SRCS
    wdgrandompickoptions.ui
    )

add_library(kritarandompickfilter MODULE ${kritarandompickfilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritarandompickfilter kritarandompickfilter.desktop)


target_link_libraries(kritarandompickfilter kritaui)

install(TARGETS kritarandompickfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritarandompickfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
