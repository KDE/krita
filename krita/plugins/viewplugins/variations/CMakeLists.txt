




########### next target ###############

set(kritavariations_PART_SRCS variations.cc dlg_variations.cc )

kde4_automoc(${kritavariations_PART_SRCS})

kde4_add_ui3_files(kritavariations_PART_SRCS wdg_variations.ui )

kde4_add_plugin(kritavariations ${kritavariations_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritavariations )

target_link_libraries(kritavariations kritaui)

install(TARGETS kritavariations  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  variations.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritavariations.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

