

set(kritanoisefilter_PART_SRCS
    noisefilter.cc
    kis_wdg_noise.cpp
    )

kde4_automoc(${kritanoisefilter_PART_SRCS})

kde4_add_ui_files(kritanoisefilter_PART_SRCS
    wdgnoiseoptions.ui
    )

kde4_add_plugin(kritanoisefilter ${kritanoisefilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritanoisefilter )

target_link_libraries(kritanoisefilter kritaui)

install(TARGETS kritanoisefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanoisefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
