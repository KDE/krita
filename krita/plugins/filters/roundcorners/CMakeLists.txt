set(kritaroundcornersfilter_PART_SRCS kis_round_corners_filter_plugin.cpp kis_round_corners_filter.cpp )

kde4_add_plugin(kritaroundcornersfilter ${kritaroundcornersfilter_PART_SRCS})



target_link_libraries(kritaroundcornersfilter kritaui)

install(TARGETS kritaroundcornersfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaroundcornersfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

