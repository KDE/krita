

set(kritaextensioncolorsfilters_PART_SRCS
    colors.cc
    kis_wdg_color_to_alpha.cc
    kis_color_to_alpha.cc
    kis_minmax_filters.cc
    )

kde4_automoc(${kritaextensioncolorsfilters_PART_SRCS})

kde4_add_ui_files(kritaextensioncolorsfilters_PART_SRCS
    wdgcolortoalphabase.ui
    )

kde4_add_plugin(kritaextensioncolorsfilters ${kritaextensioncolorsfilters_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaextensioncolorsfilters )

target_link_libraries(kritaextensioncolorsfilters kritaui)

install(TARGETS kritaextensioncolorsfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaextensioncolorsfilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
