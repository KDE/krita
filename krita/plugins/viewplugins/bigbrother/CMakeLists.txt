
set(kritabigbrother_PART_SRCS bigbrother.cc )


# kde4_add_ui_files(kritabigbrother_PART_SRCS myuifile.ui )

kde4_add_plugin(kritabigbrother ${kritabigbrother_PART_SRCS})

target_link_libraries(kritabigbrother kritaui)

install(TARGETS kritabigbrother  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  bigbrother.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritabigbrother.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

