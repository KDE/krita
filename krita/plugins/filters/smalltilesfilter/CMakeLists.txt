
set(kritasmalltilesfilter_PART_SRCS kis_small_tiles_filter_plugin.cpp kis_small_tiles_filter.cpp )


kde4_add_plugin(kritasmalltilesfilter ${kritasmalltilesfilter_PART_SRCS})



target_link_libraries(kritasmalltilesfilter kritaui)

install(TARGETS kritasmalltilesfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritasmalltilesfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


