set(kritabackgrounds_PART_SRCS
    backgrounds.cpp
    dlg_backgrounds.cpp
    wdg_backgrounds.cpp
    )

kde4_add_ui_files(kritabackgrounds_PART_SRCS
    wdg_backgrounds.ui
    )

kde4_add_plugin(kritabackgrounds ${kritabackgrounds_PART_SRCS})

target_link_libraries(kritabackgrounds kritaui)

#install(TARGETS kritabackgrounds  DESTINATION ${PLUGIN_INSTALL_DIR})

#install( FILES  backgrounds.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
#install( FILES  kritabackgrounds.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
