set(kritabmpexport_PART_SRCS
    kis_bmp_export.cpp
    )

kde4_add_ui_files(kritabmpexport_PART_SRCS )

kde4_add_plugin(kritabmpexport ${kritabmpexport_PART_SRCS})

target_link_libraries(kritabmpexport kritaui)

install(TARGETS kritabmpexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_bmp_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_bmp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

set(kritabmpimport_PART_SRCS
    kis_bmp_import.cpp
    )

kde4_add_ui_files(kritabmpimport_PART_SRCS )

kde4_add_plugin(kritabmpimport ${kritabmpimport_PART_SRCS})

target_link_libraries(kritabmpimport kritaui)

install(TARGETS kritabmpimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_bmp_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_bmp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
