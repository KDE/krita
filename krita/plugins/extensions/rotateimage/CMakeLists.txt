




########### next target ###############

set(kritarotateimage_SOURCES rotateimage.cc dlg_rotateimage.cc )


ki18n_wrap_ui(kritarotateimage_SOURCES wdg_rotateimage.ui )

add_library(kritarotateimage MODULE ${kritarotateimage_SOURCES})
kcoreaddons_desktop_to_json(kritarotateimage kritarotateimage.desktop)


target_link_libraries(kritarotateimage kritaui)

install(TARGETS kritarotateimage  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotateimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
