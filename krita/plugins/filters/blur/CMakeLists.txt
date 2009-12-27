

set(kritablurfilter_PART_SRCS
    blur.cpp
    kis_blur_filter.cpp
    kis_wdg_blur.cpp
    kis_gaussian_blur_filter.cpp
    kis_wdg_gaussian_blur.cpp
    )

kde4_add_ui_files(kritablurfilter_PART_SRCS
    wdgblur.ui
    wdg_gaussian_blur.ui
    )

kde4_add_plugin(kritablurfilter ${kritablurfilter_PART_SRCS})



target_link_libraries(kritablurfilter kritaui)

install(TARGETS kritablurfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritablurfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
