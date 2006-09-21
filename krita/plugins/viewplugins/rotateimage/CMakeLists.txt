




########### next target ###############

set(kritarotateimage_PART_SRCS rotateimage.cc dlg_rotateimage.cc )

kde4_automoc(${kritarotateimage_PART_SRCS})

kde4_add_ui_files(kritarotateimage_PART_SRCS wdg_rotateimage.ui )

kde4_add_plugin(kritarotateimage ${kritarotateimage_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritarotateimage )

target_link_libraries(kritarotateimage kritaui)

install(TARGETS kritarotateimage  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotateimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritarotateimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
