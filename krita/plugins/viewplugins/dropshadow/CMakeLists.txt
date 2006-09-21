
set(kritadropshadow_PART_SRCS kis_dropshadow.cc dlg_dropshadow.cc kis_dropshadow_plugin.cc )

kde4_automoc(${kritadropshadow_PART_SRCS})

kde4_add_ui_files(kritadropshadow_PART_SRCS wdg_dropshadow.ui )

kde4_add_plugin(kritadropshadow ${kritadropshadow_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritadropshadow )

target_link_libraries(kritadropshadow kritaui kritargb)

install(TARGETS kritadropshadow  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  dropshadow.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritadropshadow.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

