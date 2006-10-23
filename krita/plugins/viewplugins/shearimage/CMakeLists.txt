




########### next target ###############

set(kritashearimage_PART_SRCS shearimage.cc dlg_shearimage.cc )

kde4_automoc(${kritashearimage_PART_SRCS})

kde4_add_ui_files(kritashearimage_PART_SRCS wdg_shearimage.ui )

kde4_add_plugin(kritashearimage ${kritashearimage_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritashearimage )

target_link_libraries(kritashearimage kritaui)

install(TARGETS kritashearimage  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  shearimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritashearimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


