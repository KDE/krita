install( PROGRAMS  krita_flipbook.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

set(kritaflipbookimport_PART_SRCS
    kis_flipbook_import.cpp
    )

kde4_add_ui_files(kritaflipbookimport_PART_SRCS )

kde4_add_plugin(kritaflipbookimport ${kritaflipbookimport_PART_SRCS})

target_link_libraries(kritaflipbookimport kritaui)

install(TARGETS kritaflipbookimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_flipbook_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_flipbook.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
