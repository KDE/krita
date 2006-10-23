set(kritaroundcornersfilter_PART_SRCS kis_round_corners_filter_plugin.cc kis_round_corners_filter.cc )

kde4_automoc(${kritaroundcornersfilter_PART_SRCS})

kde4_add_plugin(kritaroundcornersfilter ${kritaroundcornersfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaroundcornersfilter )

target_link_libraries(kritaroundcornersfilter kritaui)

install(TARGETS kritaroundcornersfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaroundcornersfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

