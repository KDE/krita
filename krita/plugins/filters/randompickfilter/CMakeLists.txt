

set(kritarandompickfilter_PART_SRCS
    randompickfilter.cc
    kis_wdg_random_pick.cpp
    )

kde4_automoc(${kritarandompickfilter_PART_SRCS})

kde4_add_ui_files(kritarandompickfilter_PART_SRCS
    wdgrandompickoptions.ui
    )

kde4_add_plugin(kritarandompickfilter ${kritarandompickfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritarandompickfilter )

target_link_libraries(kritarandompickfilter kritaui)

install(TARGETS kritarandompickfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritarandompickfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
