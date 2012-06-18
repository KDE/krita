set(kritaoiioexport_PART_SRCS
    kis_oiio_export.cpp
    )

kde4_add_ui_files(kritaoiioexport_PART_SRCS )

kde4_add_plugin(kritaoiioexport ${kritaoiioexport_PART_SRCS})

target_link_libraries(kritaoiioexport kritaui ${OIIO_LIBRARIES})

install(TARGETS kritaoiioexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_oiio_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_oiio.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

set(kritaoiioimport_PART_SRCS
    kis_oiio_import.cpp
    )

kde4_add_ui_files(kritaoiioimport_PART_SRCS )

kde4_add_plugin(kritaoiioimport ${kritaoiioimport_PART_SRCS})

target_link_libraries(kritaoiioimport kritaui ${OIIO_LIBRARIES})

install(TARGETS kritaoiioimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_oiio_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_oiio.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
