set(kritatgaexport_PART_SRCS
    kis_tga_export.cpp
    )

kde4_add_ui_files(kritatgaexport_PART_SRCS )

kde4_add_plugin(kritatgaexport ${kritatgaexport_PART_SRCS})

target_link_libraries(kritatgaexport kritaui)

install(TARGETS kritatgaexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_tga_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
set(kritatgaimport_PART_SRCS
    kis_tga_import.cpp
    )

kde4_add_ui_files(kritatgaimport_PART_SRCS )

kde4_add_plugin(kritatgaimport ${kritatgaimport_PART_SRCS})

target_link_libraries(kritatgaimport kritaui ${KDE4_KIO_LIBS})

install(TARGETS kritatgaimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_tga_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_tga.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
