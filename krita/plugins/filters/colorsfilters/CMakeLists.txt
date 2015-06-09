

set(kritacolorsfilters_PART_SRCS
    colorsfilters.cpp
    kis_hsv_adjustment_filter.cpp
    virtual_channel_info.cpp
    kis_perchannel_filter.cpp
    kis_brightness_contrast_filter.cpp
    kis_color_balance_filter.cpp
    kis_desaturate_filter.cpp
    )

ki18n_wrap_ui(kritacolorsfilters_PART_SRCS
    wdg_perchannel.ui
    wdg_color_balance.ui
    wdg_brightness_contrast.ui
    wdg_hsv_adjustment.ui
    wdg_desaturate.ui
)

add_library(kritacolorsfilters MODULE ${kritacolorsfilters_PART_SRCS})
kcoreaddons_desktop_to_json(kritacolorsfilters kritacolorsfilter.desktop)



target_link_libraries(kritacolorsfilters kritaui)

install(TARGETS kritacolorsfilters  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacolorsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
