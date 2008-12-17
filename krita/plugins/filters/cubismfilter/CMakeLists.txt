




########### next target ###############

set(kritacubismfilter_PART_SRCS kis_cubism_filter_plugin.cc kis_cubism_filter.cc kis_polygon.cc )


kde4_add_plugin(kritacubismfilter ${kritacubismfilter_PART_SRCS})



target_link_libraries(kritacubismfilter koguiutils kritaimage kritaui ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritacubismfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacubismfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

