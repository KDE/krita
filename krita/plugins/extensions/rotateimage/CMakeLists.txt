




########### next target ###############

set(kritarotateimage_PART_SRCS rotateimage.cc dlg_rotateimage.cc )


ki18n_wrap_ui(kritarotateimage_PART_SRCS wdg_rotateimage.ui )

add_library(kritarotateimage MODULE ${kritarotateimage_PART_SRCS})
kcoreaddons_desktop_to_json(kritarotateimage kritarotateimage.desktop)


target_link_libraries(kritarotateimage kritaui)

install(TARGETS kritarotateimage  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotateimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
