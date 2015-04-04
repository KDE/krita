

set(kritawavefilter_PART_SRCS
    wavefilter.cpp
    kis_wdg_wave.cpp
    )


ki18n_wrap_ui(kritawavefilter_PART_SRCS
    wdgwaveoptions.ui
    )

add_library(kritawavefilter MODULE ${kritawavefilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritawavefilter kritawavefilter.desktop)


target_link_libraries(kritawavefilter kritaui)

install(TARGETS kritawavefilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritawavefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
