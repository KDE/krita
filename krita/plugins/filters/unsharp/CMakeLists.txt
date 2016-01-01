add_subdirectory( tests )

set(kritaunsharpfilter_SOURCES
    unsharp.cpp
    kis_wdg_unsharp.cpp
    kis_unsharp_filter.cpp
    )

ki18n_wrap_ui(kritaunsharpfilter_SOURCES
    wdgunsharp.ui
    )

add_library(kritaunsharpfilter MODULE ${kritaunsharpfilter_SOURCES})

target_link_libraries(kritaunsharpfilter kritaui)

install(TARGETS kritaunsharpfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
