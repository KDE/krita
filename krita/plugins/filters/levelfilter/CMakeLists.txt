set(kritalevelfilter_PART_SRCS
    levelfilter.cpp
    kis_level_filter.cpp
    )

ki18n_wrap_ui(kritalevelfilter_PART_SRCS
    wdg_level.ui
    )

add_library(kritalevelfilter MODULE ${kritalevelfilter_PART_SRCS})



target_link_libraries(kritalevelfilter kritaui)

install(TARGETS kritalevelfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritalevelfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
