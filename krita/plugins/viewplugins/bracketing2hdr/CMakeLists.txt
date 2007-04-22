
set(kritabracketing2hdr_PART_SRCS bracketing2hdr.cc )

kde4_automoc(${kritabracketing2hdr_PART_SRCS})

kde4_add_ui_files(kritabracketing2hdr_PART_SRCS wdgbracketing2hdr.ui )

kde4_add_plugin(kritabracketing2hdr ${kritabracketing2hdr_PART_SRCS})

target_link_libraries(kritabracketing2hdr kritaui)

install(TARGETS kritabracketing2hdr  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  bracketing2hdr.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritabracketing2hdr.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

