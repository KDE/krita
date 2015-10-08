add_subdirectory( tests )

set(kritaunsharpfilter_PART_SRCS
    unsharp.cpp
    kis_wdg_unsharp.cpp
    kis_unsharp_filter.cpp
    )

ki18n_wrap_ui(kritaunsharpfilter_PART_SRCS
    wdgunsharp.ui
    )

add_library(kritaunsharpfilter MODULE ${kritaunsharpfilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritaunsharpfilter kritaunsharpfilter.desktop)


target_link_libraries(kritaunsharpfilter kritaui)

install(TARGETS kritaunsharpfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
