
set(kritahistogram_PART_SRCS histogram.cc dlg_histogram.cc kis_histogram_widget.cc )


ki18n_wrap_ui(kritahistogram_PART_SRCS wdghistogram.ui )

add_library(kritahistogram MODULE ${kritahistogram_PART_SRCS})
kcoreaddons_desktop_to_json(kritahistogram kritahistogram.desktop)


target_link_libraries(kritahistogram kritaui)

install(TARGETS kritahistogram  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  histogram.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritahistogram.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

