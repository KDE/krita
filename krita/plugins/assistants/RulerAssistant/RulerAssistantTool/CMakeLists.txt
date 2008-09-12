include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RulerAssistantCommon)

set(kritarulerassistanttool_PART_SRCS
    ruler_assistant_tool.cc
    kis_ruler_assistant_tool.cc
    )

# kde4_add_ui_files(kritarulerassistanttool_PART_SRCS RulerAssistantToolOptions.ui )

kde4_add_plugin(kritarulerassistanttool ${kritarulerassistanttool_PART_SRCS})



target_link_libraries(kritarulerassistanttool kritaui ${FLAKE_LIBS} KritaRulerAssistantCommon)

install(TARGETS kritarulerassistanttool  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

# install( FILES  tool_rulerassistanttool.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritarulerassistanttool.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


