




########### next target ###############

set(kritaseparatechannels_PART_SRCS kis_channel_separator.cc dlg_separate.cc kis_separate_channels_plugin.cc )

kde4_automoc(${kritaseparatechannels_PART_SRCS})

kde4_add_ui3_files(kritaseparatechannels_PART_SRCS wdg_separations.ui )

kde4_add_plugin(kritaseparatechannels ${kritaseparatechannels_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaseparatechannels )

target_link_libraries(kritaseparatechannels kritaui)

install(TARGETS kritaseparatechannels  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  imageseparate.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaseparatechannels.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
