

set(kritacolorsfilters_PART_SRCS
    colorsfilters.cpp
    kis_hsv_adjustment_filter.cpp
    kis_perchannel_filter.cpp
    kis_brightness_contrast_filter.cpp
    )

kde4_add_ui_files(kritacolorsfilters_PART_SRCS
    wdg_perchannel.ui
    wdg_brightness_contrast.ui
    wdg_hsv_adjustment.ui
    )

kde4_add_plugin(kritacolorsfilters ${kritacolorsfilters_PART_SRCS})



target_link_libraries(kritacolorsfilters kritaui)

install(TARGETS kritacolorsfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacolorsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
