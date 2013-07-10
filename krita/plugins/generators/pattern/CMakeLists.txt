set(kritapatterngenerator_PART_SRCS
    patterngenerator.cpp
    kis_wdg_pattern.cpp
    )


kde4_add_ui_files(kritapatterngenerator_PART_SRCS
    wdgpatternoptions.ui
    )

kde4_add_plugin(kritapatterngenerator ${kritapatterngenerator_PART_SRCS})


target_link_libraries(kritapatterngenerator kritaui)

install(TARGETS kritapatterngenerator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapatterngenerator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
