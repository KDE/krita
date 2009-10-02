

set(kritaextensioncolorsfilters_PART_SRCS
    colors.cpp
    kis_wdg_color_to_alpha.cpp
    kis_color_to_alpha.cpp
    kis_minmax_filters.cpp
    )


kde4_add_ui_files(kritaextensioncolorsfilters_PART_SRCS
    wdgcolortoalphabase.ui
    )

kde4_add_plugin(kritaextensioncolorsfilters ${kritaextensioncolorsfilters_PART_SRCS})



target_link_libraries(kritaextensioncolorsfilters kritaui)

install(TARGETS kritaextensioncolorsfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaextensioncolorsfilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
