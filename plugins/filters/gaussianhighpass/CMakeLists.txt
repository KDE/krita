set(kritahighpassfilter_SOURCES
    gaussianhighpass.cpp
    wdg_gaussianhighpass.cpp
    gaussianhighpass_filter.cpp
    )

ki18n_wrap_ui(kritahighpassfilter_SOURCES
    wdggaussianhighpass.ui
    )

kis_add_library(kritagaussianhighpassfilter MODULE ${kritahighpassfilter_SOURCES})

target_link_libraries(kritagaussianhighpassfilter kritaui)

install(TARGETS kritagaussianhighpassfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
