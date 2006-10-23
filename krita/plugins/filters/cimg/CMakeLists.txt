set(kritacimg_PART_SRCS
    kis_cimg_filter.cc
    kis_cimg_plugin.cc
    kis_cimgconfig_widget.cc
    )

kde4_automoc(${kritacimg_PART_SRCS})

kde4_add_ui_files(kritacimg_PART_SRCS wdg_cimg.ui)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

if(Q_WS_MAC)
add_definitions("-Dcimg_display_type=0")
endif(Q_WS_MAC)

kde4_add_plugin(kritacimg ${kritacimg_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritacimg )

target_link_libraries(kritacimg kritaui)

install(TARGETS kritacimg  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritacimg.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

