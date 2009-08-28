

set(kritablurfilter_PART_SRCS
    blur.cc
    kis_blur_filter.cc
    kis_wdg_blur.cc
    )

kde4_add_ui_files(kritablurfilter_PART_SRCS
    wdgblur.ui
    )

kde4_add_plugin(kritablurfilter ${kritablurfilter_PART_SRCS})



target_link_libraries(kritablurfilter kritaui koaspectbutton)

install(TARGETS kritablurfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritablurfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
