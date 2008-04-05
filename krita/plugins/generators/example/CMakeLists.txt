set(kritacolorgenerator_PART_SRCS
    colorgenerator.cpp
    kis_wdg_color.cpp
    )


kde4_add_ui_files(kritacolorgenerator_PART_SRCS
    wdgcoloroptions.ui
    )

kde4_add_plugin(kritacolorgenerator ${kritacolorgenerator_PART_SRCS})


target_link_libraries(kritacolorgenerator kritaui)

install(TARGETS kritacolorgenerator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacolorgenerator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
