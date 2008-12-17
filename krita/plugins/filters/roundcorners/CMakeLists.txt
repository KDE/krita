set(kritaroundcornersfilter_PART_SRCS kis_round_corners_filter_plugin.cc kis_round_corners_filter.cc )

kde4_add_plugin(kritaroundcornersfilter ${kritaroundcornersfilter_PART_SRCS})



target_link_libraries(kritaroundcornersfilter koguiutils kritaimage kritaui ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritaroundcornersfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritaroundcornersfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

