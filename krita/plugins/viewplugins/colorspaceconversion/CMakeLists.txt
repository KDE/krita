
set(kritacolorspaceconversion_PART_SRCS colorspaceconversion.cc dlg_colorspaceconversion.cc )

kde4_automoc(${kritacolorspaceconversion_PART_SRCS})

kde4_add_ui_files(kritacolorspaceconversion_PART_SRCS wdgconvertcolorspace.ui )

kde4_add_plugin(kritacolorspaceconversion ${kritacolorspaceconversion_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacolorspaceconversion )

target_link_libraries(kritacolorspaceconversion kritaui)

install(TARGETS kritacolorspaceconversion  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  colorspaceconversion.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritacolorspaceconversion.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

