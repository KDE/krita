set(kritahighpassfilter_SOURCES
    guassianhighpass.cpp
    wdg_guassianhighpass.cpp
    guassianhighpass_filter.cpp
    )

ki18n_wrap_ui(kritahighpassfilter_SOURCES
    wdgguassianhighpass.ui
    )

add_library(kritaguassianhighpassfilter MODULE ${kritahighpassfilter_SOURCES})

target_link_libraries(kritaguassianhighpassfilter kritaui)

install(TARGETS kritaguassianhighpassfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
