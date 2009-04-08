set(kritabmpexport_PART_SRCS
    kis_bmp_export.cpp
    )

kde4_add_ui_files(kritabmpexport_PART_SRCS )

kde4_add_plugin(kritabmpexport WITH_PREFIX ${kritabmpexport_PART_SRCS})

target_link_libraries(kritabmpexport kritaui ${bmp_LIBRARIES})

install(TARGETS kritabmpexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_bmp_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_bmp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
