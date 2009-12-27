add_subdirectory( pics )

set(kritaimagesize_PART_SRCS
    imagesize.cc
    dlg_imagesize.cc
    dlg_canvassize.cc
    dlg_layersize.cc
    kcanvaspreview.cc
    multilock_button.cc
    )

kde4_add_ui_files(kritaimagesize_PART_SRCS
    wdg_imagescale.ui
    wdg_canvassize.ui
    wdg_layersize.ui
    )

kde4_add_plugin(kritaimagesize ${kritaimagesize_PART_SRCS})



target_link_libraries(kritaimagesize kritaui)

install(TARGETS kritaimagesize  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  imagesize.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaimagesize.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
