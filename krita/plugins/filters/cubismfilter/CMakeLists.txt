




########### next target ###############

set(kritacubismfilter_PART_SRCS kis_cubism_filter_plugin.cpp kis_cubism_filter.cpp kis_polygon.cpp )


kde4_add_plugin(kritacubismfilter ${kritacubismfilter_PART_SRCS})



target_link_libraries(kritacubismfilter kritaui)

install(TARGETS kritacubismfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacubismfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

