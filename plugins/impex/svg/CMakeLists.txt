add_subdirectory(tests)

set(kritasvgimport_SOURCES
    kis_svg_import.cc
    )

add_library(kritasvgimport MODULE ${kritasvgimport_SOURCES})

add_definitions(${SVG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritasvgimport kritaui )

install(TARGETS kritasvgimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_svg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
