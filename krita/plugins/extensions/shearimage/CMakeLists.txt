
set(kritashearimage_SOURCES shearimage.cc dlg_shearimage.cc )


ki18n_wrap_ui(kritashearimage_SOURCES wdg_shearimage.ui )

add_library(kritashearimage MODULE ${kritashearimage_SOURCES})
kcoreaddons_desktop_to_json(kritashearimage kritashearimage.desktop)


target_link_libraries(kritashearimage kritaui)

install(TARGETS kritashearimage  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  shearimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
