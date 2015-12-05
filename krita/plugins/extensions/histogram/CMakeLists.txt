
set(kritahistogram_SOURCES histogram.cc dlg_histogram.cc kis_histogram_widget.cc )


ki18n_wrap_ui(kritahistogram_SOURCES wdghistogram.ui )

add_library(kritahistogram MODULE ${kritahistogram_SOURCES})
kcoreaddons_desktop_to_json(kritahistogram kritahistogram.desktop)


target_link_libraries(kritahistogram kritaui)

install(TARGETS kritahistogram  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  histogram.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
