

set(kritacolorsfilters_PART_SRCS
    colorsfilters.cc
    kis_perchannel_filter.cc
    kis_brightness_contrast_filter.cc
    )

kde4_automoc(${kritacolorsfilters_PART_SRCS})

kde4_add_ui_files(kritacolorsfilters_PART_SRCS
    wdg_perchannel.ui
    wdg_brightness_contrast.ui
    )

kde4_add_plugin(kritacolorsfilters ${kritacolorsfilters_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorsfilters )

target_link_libraries(kritacolorsfilters kritaui)

install(TARGETS kritacolorsfilters  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacolorsfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
