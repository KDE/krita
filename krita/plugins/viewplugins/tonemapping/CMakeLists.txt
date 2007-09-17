include_directories(lib/ )

set(kritatonemapping_PART_SRCS tonemapping.cc lib/kis_tone_mapping_operator.cc lib/kis_tone_mapping_operators_registry.cc )


# kde4_add_ui_files(kritatonemapping_PART_SRCS myuifile.ui )

kde4_add_plugin(kritatonemapping ${kritatonemapping_PART_SRCS})

target_link_libraries(kritatonemapping kritaui)

install(TARGETS kritatonemapping  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tonemapping.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritatonemapping.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

