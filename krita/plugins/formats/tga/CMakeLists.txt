set(kritatgaexport_SOURCES
    kis_tga_export.cpp
    )

ki18n_wrap_ui(kritatgaexport_SOURCES )

add_library(kritatgaexport MODULE ${kritatgaexport_SOURCES})

target_link_libraries(kritatgaexport kritaui)

install(TARGETS kritatgaexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritatgaimport_SOURCES
    kis_tga_import.cpp
    )

ki18n_wrap_ui(kritatgaimport_SOURCES )

add_library(kritatgaimport MODULE ${kritatgaimport_SOURCES})

target_link_libraries(kritatgaimport kritaui )

install(TARGETS kritatgaimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_tga.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
