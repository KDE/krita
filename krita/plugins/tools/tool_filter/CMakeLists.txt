

set(kritatoolfilter_PART_SRCS
    kis_filterop.cc
    kis_tool_filter.cc
    tool_filter.cc
    )

kde4_automoc(${kritatoolfilter_PART_SRCS})

kde4_add_plugin(kritatoolfilter ${kritatoolfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolfilter )

target_link_libraries(kritatoolfilter kritaui)

install(TARGETS kritatoolfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_filter.png  	tool_filter_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


