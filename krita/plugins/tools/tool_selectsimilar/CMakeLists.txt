

set(kritatoolselectsimilar_PART_SRCS
    selectsimilar.cc
    kis_tool_selectsimilar.cc
    )

kde4_automoc(${kritatoolselectsimilar_PART_SRCS})

kde4_add_plugin(kritatoolselectsimilar ${kritatoolselectsimilar_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolselectsimilar )

target_link_libraries(kritatoolselectsimilar kritaui)

install(TARGETS kritatoolselectsimilar  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_similar_selection.png  	tool_similar_selection_plus_cursor.png  	tool_similar_selection_minus_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolselectsimilar.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


