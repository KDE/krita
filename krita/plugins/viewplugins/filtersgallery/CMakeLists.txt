

set(kritafiltersgallery_PART_SRCS
    filters_gallery.cc
    kis_dlg_filtersgallery.cc
    )

kde4_automoc(${kritafiltersgallery_PART_SRCS})

kde4_add_ui_files(kritafiltersgallery_PART_SRCS kis_wdg_filtersgallery.ui)

kde4_add_plugin(kritafiltersgallery ${kritafiltersgallery_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritafiltersgallery )

target_link_libraries(kritafiltersgallery kritaui)

install(TARGETS kritafiltersgallery  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafiltersgallery.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritafiltersgallery.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
