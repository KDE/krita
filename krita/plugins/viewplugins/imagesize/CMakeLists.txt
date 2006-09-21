

set(kritaimagesize_PART_SRCS
    imagesize.cc
    dlg_imagesize.cc
    dlg_layersize.cc
    )

kde4_automoc(${kritaimagesize_PART_SRCS})

kde4_add_ui_files(kritaimagesize_PART_SRCS
    wdg_resolution.ui
    wdg_imagesize.ui
    wdg_layersize.ui
    )

kde4_add_plugin(kritaimagesize ${kritaimagesize_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritaimagesize )

target_link_libraries(kritaimagesize kritaui)

install(TARGETS kritaimagesize  DESTINATION ${PLUGIN_INSTALL_DIR})

