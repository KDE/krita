set(kritalevelfilter_PART_SRCS
    levelfilter.cpp
    kis_level_filter.cpp
    )

ki18n_wrap_ui(kritalevelfilter_PART_SRCS
    wdg_level.ui
    )

add_library(kritalevelfilter MODULE ${kritalevelfilter_PART_SRCS})
kcoreaddons_desktop_to_json(kritalevelfilter kritalevelfilter.desktop)


target_link_libraries(kritalevelfilter kritaui)

install(TARGETS kritalevelfilter  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
