

set(kritablurfilter_PART_SRCS
    blur.cc
    kis_blur_filter.cc
    kis_wdg_blur.cc
    )

kde4_automoc(${kritablurfilter_PART_SRCS})

kde4_add_ui_files(kritablurfilter_PART_SRCS
    wdgblur.ui
    )

kde4_add_plugin(kritablurfilter ${kritablurfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritablurfilter )

target_link_libraries(kritablurfilter kritaui)

install(TARGETS kritablurfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritablurfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
