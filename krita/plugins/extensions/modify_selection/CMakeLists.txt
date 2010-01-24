

set(kritamodifyselection_PART_SRCS
    dlg_grow_selection.cc
    dlg_shrink_selection.cc
    dlg_border_selection.cc
    dlg_feather_selection.cc
    modify_selection.cc
    )


kde4_add_ui_files(kritamodifyselection_PART_SRCS
    wdg_grow_selection.ui
    wdg_shrink_selection.ui
    wdg_border_selection.ui
    wdg_feather_selection.ui
    )

kde4_add_plugin(kritamodifyselection ${kritamodifyselection_PART_SRCS})



target_link_libraries(kritamodifyselection kritaui)

install(TARGETS kritamodifyselection  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  modify_selection.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritamodifyselection.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

