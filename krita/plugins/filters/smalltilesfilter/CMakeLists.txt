
set(kritasmalltilesfilter_PART_SRCS kis_small_tiles_filter_plugin.cc kis_small_tiles_filter.cc )


kde4_add_plugin(kritasmalltilesfilter ${kritasmalltilesfilter_PART_SRCS})



target_link_libraries(kritasmalltilesfilter koguiutils kritaimage kritaui ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kritasmalltilesfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasmalltilesfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


