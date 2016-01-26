set(kritacolorsfilters_SOURCES
    colorsfilters.cpp
    kis_hsv_adjustment_filter.cpp
    virtual_channel_info.cpp
    kis_perchannel_filter.cpp
    kis_brightness_contrast_filter.cpp
    kis_color_balance_filter.cpp
    kis_desaturate_filter.cpp
    )

ki18n_wrap_ui(kritacolorsfilters_SOURCES
    wdg_perchannel.ui
    wdg_color_balance.ui
    wdg_brightness_contrast.ui
    wdg_hsv_adjustment.ui
    wdg_desaturate.ui
)

add_library(kritacolorsfilters MODULE ${kritacolorsfilters_SOURCES})
target_link_libraries(kritacolorsfilters kritaui)
install(TARGETS kritacolorsfilters  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
