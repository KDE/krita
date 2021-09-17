set(kritacolorsfilters_SOURCES
    colorsfilters.cpp
    kis_hsv_adjustment_filter.cpp
    virtual_channel_info.cpp
    kis_multichannel_filter_base.cpp
    kis_multichannel_utils.cpp
    kis_perchannel_filter.cpp
    kis_cross_channel_filter.cpp
    kis_color_balance_filter.cpp
    kis_desaturate_filter.cpp
    )

ki18n_wrap_ui(kritacolorsfilters_SOURCES
    wdg_perchannel.ui
    wdg_color_balance.ui
    wdg_hsv_adjustment.ui
    wdg_desaturate.ui
)

add_library(kritacolorsfilters MODULE ${kritacolorsfilters_SOURCES})
target_link_libraries(kritacolorsfilters kritaui)
install(TARGETS kritacolorsfilters  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
