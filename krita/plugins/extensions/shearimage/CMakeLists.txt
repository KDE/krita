
set(kritashearimage_PART_SRCS shearimage.cc dlg_shearimage.cc )


ki18n_wrap_ui(kritashearimage_PART_SRCS wdg_shearimage.ui )

add_library(kritashearimage MODULE ${kritashearimage_PART_SRCS})



target_link_libraries(kritashearimage kritaui)

install(TARGETS kritashearimage  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  shearimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritashearimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


