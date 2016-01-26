add_subdirectory( pics )

set(kritaimagesize_SOURCES
    imagesize.cc
    dlg_imagesize.cc
    dlg_canvassize.cc
    dlg_layersize.cc
    kcanvaspreview.cc
    )

ki18n_wrap_ui(kritaimagesize_SOURCES
    wdg_imagesize.ui
    wdg_canvassize.ui
    wdg_layersize.ui
    )

add_library(kritaimagesize MODULE ${kritaimagesize_SOURCES})
target_link_libraries(kritaimagesize kritaui)
install(TARGETS kritaimagesize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  imagesize.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
