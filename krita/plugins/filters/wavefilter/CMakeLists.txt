

set(kritawavefilter_PART_SRCS
    wavefilter.cpp
    kis_wdg_wave.cpp
    )


kde4_add_ui_files(kritawavefilter_PART_SRCS
    wdgwaveoptions.ui
    )

kde4_add_plugin(kritawavefilter ${kritawavefilter_PART_SRCS})



target_link_libraries(kritawavefilter kritaui)

install(TARGETS kritawavefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritawavefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
