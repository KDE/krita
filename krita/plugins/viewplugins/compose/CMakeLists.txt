set(kritalayercompose_PART_SRCS dlg_compose.cpp kis_layer_compose_plugin.cpp )

kde4_add_ui_files(kritalayercompose_PART_SRCS wdg_compose.ui )
kde4_add_plugin(kritalayercompose ${kritalayercompose_PART_SRCS})

target_link_libraries(kritalayercompose kritaui)

install(TARGETS kritalayercompose  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  layercompose.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritalayercompose.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
