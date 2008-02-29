set(kritanoisegenerator_PART_SRCS
    noisegenerator.cpp
    kis_wdg_noise.cpp
    )


kde4_add_ui_files(kritanoisegenerator_PART_SRCS
    wdgnoiseoptions.ui
    )

kde4_add_plugin(kritanoisegenerator ${kritanoisegenerator_PART_SRCS})


target_link_libraries(kritanoisegenerator kritaui)

install(TARGETS kritanoisegenerator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritanoisegenerator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
