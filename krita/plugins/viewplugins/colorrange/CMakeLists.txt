
set(kritacolorrange_PART_SRCS colorrange.cc dlg_colorrange.cc )

kde4_automoc(${kritacolorrange_PART_SRCS})

kde4_add_ui_files(kritacolorrange_PART_SRCS wdg_colorrange.ui )

kde4_add_plugin(kritacolorrange ${kritacolorrange_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorrange )

target_link_libraries(kritacolorrange kritaui)

install(TARGETS kritacolorrange  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  colorrange.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritacolorrange.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


