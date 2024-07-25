set(kritablurfilter_SOURCES
    blur.cpp
    kis_blur_filter.cpp
    kis_wdg_blur.cpp
    kis_gaussian_blur_filter.cpp
    kis_wdg_gaussian_blur.cpp
    kis_motion_blur_filter.cpp
    kis_wdg_motion_blur.cpp
    kis_lens_blur_filter.cpp
    kis_wdg_lens_blur.cpp
    )

ki18n_wrap_ui(kritablurfilter_SOURCES
    wdgblur.ui
    wdg_gaussian_blur.ui
    wdg_motion_blur.ui
    wdg_lens_blur.ui
    )

kis_add_library(kritablurfilter MODULE ${kritablurfilter_SOURCES})
target_link_libraries(kritablurfilter kritaui)
install(TARGETS kritablurfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
