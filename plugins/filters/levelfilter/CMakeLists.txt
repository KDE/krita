set(kritalevelfilter_SOURCES
    levelfilter.cpp
    kis_level_filter.cpp
    )
ki18n_wrap_ui(kritalevelfilter_SOURCES
    wdg_level.ui
    )
kis_add_library(kritalevelfilter MODULE ${kritalevelfilter_SOURCES})
target_link_libraries(kritalevelfilter kritaui)
install(TARGETS kritalevelfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
