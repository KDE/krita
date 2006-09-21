
set(kritaperftest_PART_SRCS perftest.cc dlg_perftest.cc )

kde4_automoc(${kritaperftest_PART_SRCS})

kde4_add_ui_files(kritaperftest_PART_SRCS wdg_perftest.ui )

kde4_add_plugin(kritaperftest ${kritaperftest_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaperftest )

target_link_libraries(kritaperftest kritaui)

install(TARGETS kritaperftest  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  perftest.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaperftest.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


