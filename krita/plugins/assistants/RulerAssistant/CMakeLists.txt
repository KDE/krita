include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RulerAssistantCommon)

set(kritarulerassistanttool_PART_SRCS
    RulerAssistant.cc
    ruler_assistant_tool.cc
    kis_ruler_assistant_tool.cc
    )

kde4_add_ui_files(kritarulerassistanttool_PART_SRCS AssistantsToolOptions.ui )

kde4_add_plugin(kritarulerassistanttool ${kritarulerassistanttool_PART_SRCS})

target_link_libraries(kritarulerassistanttool kritaui flake ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} )

install(TARGETS kritarulerassistanttool  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES krita_tool_ruler_assistant.png   DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritarulerassistanttool.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
