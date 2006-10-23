
set(kritahistogram_PART_SRCS histogram.cc dlg_histogram.cc kis_histogram_widget.cc )

kde4_automoc(${kritahistogram_PART_SRCS})

kde4_add_ui_files(kritahistogram_PART_SRCS wdghistogram.ui )

kde4_add_plugin(kritahistogram ${kritahistogram_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistogram )

target_link_libraries(kritahistogram kritaui)

install(TARGETS kritahistogram  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  histogram.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritahistogram.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

