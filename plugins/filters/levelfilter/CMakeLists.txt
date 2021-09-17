set(kritalevelfilter_SOURCES
    KisLevelsFilterPlugin.cpp
    KisLevelsFilter.cpp
    KisLevelsConfigWidget.cpp
    KisLevelsFilterConfiguration.cpp
    ../colorsfilters/kis_multichannel_utils.cpp
    ../colorsfilters/virtual_channel_info.cpp
    )
ki18n_wrap_ui(kritalevelfilter_SOURCES
    KisLevelsConfigWidget.ui
    )
add_library(kritalevelfilter MODULE ${kritalevelfilter_SOURCES})
target_link_libraries(kritalevelfilter kritaui)
install(TARGETS kritalevelfilter  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
