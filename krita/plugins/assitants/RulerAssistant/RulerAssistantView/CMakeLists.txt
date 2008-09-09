
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RulerAssistantCommon)

set(kritarulerassistantview_PART_SRCS rulerassistantview.cc )


# kde4_add_ui_files(kritarulerassistantview_PART_SRCS myuifile.ui )

kde4_add_plugin(kritarulerassistantview ${kritarulerassistantview_PART_SRCS})

target_link_libraries(kritarulerassistantview kritaui KritaRulerAssistantCommon)

install(TARGETS kritarulerassistantview  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rulerassistantview.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritarulerassistantview.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

