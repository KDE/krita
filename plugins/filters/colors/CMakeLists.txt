set(kritaextensioncolorsfilters_SOURCES
    colors.cpp
    kis_wdg_color_to_alpha.cpp
    kis_color_to_alpha.cpp
    kis_minmax_filters.cpp
    )

ki18n_wrap_ui(kritaextensioncolorsfilters_SOURCES
    wdgcolortoalphabase.ui
    )

add_library(kritaextensioncolorsfilters MODULE ${kritaextensioncolorsfilters_SOURCES})
target_link_libraries(kritaextensioncolorsfilters kritaui)
install(TARGETS kritaextensioncolorsfilters  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
