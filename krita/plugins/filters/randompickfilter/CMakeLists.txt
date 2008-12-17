

set(kritarandompickfilter_PART_SRCS
    randompickfilter.cc
    kis_wdg_random_pick.cpp
    )


kde4_add_ui_files(kritarandompickfilter_PART_SRCS
    wdgrandompickoptions.ui
    )

kde4_add_plugin(kritarandompickfilter ${kritarandompickfilter_PART_SRCS})



target_link_libraries(kritarandompickfilter koguiutils kritaimage kritaui ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritarandompickfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritarandompickfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
