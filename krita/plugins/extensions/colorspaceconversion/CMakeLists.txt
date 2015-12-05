
set(kritacolorspaceconversion_SOURCES colorspaceconversion.cc dlg_colorspaceconversion.cc )


ki18n_wrap_ui(kritacolorspaceconversion_SOURCES wdgconvertcolorspace.ui )

add_library(kritacolorspaceconversion MODULE ${kritacolorspaceconversion_SOURCES})
kcoreaddons_desktop_to_json(kritacolorspaceconversion kritacolorspaceconversion.desktop)


target_link_libraries(kritacolorspaceconversion kritaui)

install(TARGETS kritacolorspaceconversion  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  colorspaceconversion.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
