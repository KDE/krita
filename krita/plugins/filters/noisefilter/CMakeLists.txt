

set(kritanoisefilter_PART_SRCS
    noisefilter.cpp
    kis_wdg_noise.cpp
    )


kde4_add_ui_files(kritanoisefilter_PART_SRCS
    wdgnoiseoptions.ui
    )

kde4_add_plugin(kritanoisefilter ${kritanoisefilter_PART_SRCS})



target_link_libraries(kritanoisefilter kritaui)

install(TARGETS kritanoisefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanoisefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
