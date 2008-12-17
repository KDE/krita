
set(kritapixelizefilter_PART_SRCS kis_pixelize_filter_plugin.cc kis_pixelize_filter.cc )


kde4_add_plugin(kritapixelizefilter ${kritapixelizefilter_PART_SRCS})



target_link_libraries(kritapixelizefilter koguiutils kritaimage kritaui ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritapixelizefilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritapixelizefilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


