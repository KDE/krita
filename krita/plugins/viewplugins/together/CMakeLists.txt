
set(kritatogether_PART_SRCS together.cc )


# kde4_add_ui_files(kritatogether_PART_SRCS myuifile.ui )

kde4_add_plugin(kritatogether ${kritatogether_PART_SRCS})

target_link_libraries(kritatogether kritaui)

install(TARGETS kritatogether  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  together.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritatogether.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

