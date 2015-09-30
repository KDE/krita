add_subdirectory( pics )

set(kritaimagesize_PART_SRCS
    imagesize.cc
    dlg_imagesize.cc
    dlg_canvassize.cc
    dlg_layersize.cc
    kcanvaspreview.cc
    )

ki18n_wrap_ui(kritaimagesize_PART_SRCS
    wdg_imagesize.ui
    wdg_canvassize.ui
    wdg_layersize.ui
    )

add_library(kritaimagesize MODULE ${kritaimagesize_PART_SRCS})
kcoreaddons_desktop_to_json(kritaimagesize kritaimagesize.desktop)


target_link_libraries(kritaimagesize kritaui)

install(TARGETS kritaimagesize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  imagesize.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
