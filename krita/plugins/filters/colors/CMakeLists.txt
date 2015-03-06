

set(kritaextensioncolorsfilters_PART_SRCS
    colors.cpp
    kis_wdg_color_to_alpha.cpp
    kis_color_to_alpha.cpp
    kis_minmax_filters.cpp
    )


ki18n_wrap_ui(kritaextensioncolorsfilters_PART_SRCS
    wdgcolortoalphabase.ui
    )

add_library(kritaextensioncolorsfilters MODULE ${kritaextensioncolorsfilters_PART_SRCS})



target_link_libraries(kritaextensioncolorsfilters kritaui)

install(TARGETS kritaextensioncolorsfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaextensioncolorsfilters.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
