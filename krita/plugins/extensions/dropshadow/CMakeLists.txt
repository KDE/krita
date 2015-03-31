
set(kritadropshadow_PART_SRCS kis_dropshadow.cc dlg_dropshadow.cc kis_dropshadow_plugin.cc )


ki18n_wrap_ui(kritadropshadow_PART_SRCS wdg_dropshadow.ui )

add_library(kritadropshadow MODULE ${kritadropshadow_PART_SRCS})
kcoreaddons_desktop_to_json(kritadropshadow kritadropshadow.desktop)


target_link_libraries(kritadropshadow kritaui)

install(TARGETS kritadropshadow  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  dropshadow.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritadropshadow.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

