
set(KRITA_DIGITALMIXER_PART_SRCS digitalmixer.cc digitalmixer_dock.cc )

# kde4_add_ui_files(kritadigitalmixer_PART_SRCS myuifile.ui )

kde4_add_plugin(kritadigitalmixer ${KRITA_DIGITALMIXER_PART_SRCS})

target_link_libraries(kritadigitalmixer kritaui)

install(TARGETS kritadigitalmixer  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_digitalmixer.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

