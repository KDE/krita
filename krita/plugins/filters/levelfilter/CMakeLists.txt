

set(kritalevelfilter_PART_SRCS
    levelfilter.cpp
    kis_level_filter.cpp
    kgradientslider.cpp
    )

kde4_add_ui_files(kritalevelfilter_PART_SRCS
    wdg_level.ui
    )

kde4_add_plugin(kritalevelfilter ${kritalevelfilter_PART_SRCS})



target_link_libraries(kritalevelfilter kritaui)

install(TARGETS kritalevelfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritalevelfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
