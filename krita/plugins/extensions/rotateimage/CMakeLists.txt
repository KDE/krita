




########### next target ###############

set(kritarotateimage_PART_SRCS rotateimage.cc dlg_rotateimage.cc )


ki18n_wrap_ui(kritarotateimage_PART_SRCS wdg_rotateimage.ui )

add_library(kritarotateimage MODULE ${kritarotateimage_PART_SRCS})



target_link_libraries(kritarotateimage kritaui)

install(TARGETS kritarotateimage  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  rotateimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritarotateimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
