set(kritagifexport_SOURCES
    kis_gif_export.cpp
    qgiflibhandler.cpp
    )

ki18n_wrap_ui(kritagifexport_SOURCES )

add_library(kritagifexport MODULE ${kritagifexport_SOURCES})

target_link_libraries(kritagifexport kritaui kritaimpex gif)

install(TARGETS kritagifexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritagifimport_SOURCES
    kis_gif_import.cpp
    qgiflibhandler.cpp
    )

ki18n_wrap_ui(kritagifimport_SOURCES )

add_library(kritagifimport MODULE ${kritagifimport_SOURCES})

target_link_libraries(kritagifimport kritaui gif)

install(TARGETS kritagifimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_gif.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
