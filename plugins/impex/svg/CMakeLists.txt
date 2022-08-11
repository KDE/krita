add_subdirectory(tests)

set(kritasvgimport_SOURCES
    kis_svg_import.cc

    )

kis_add_library(kritasvgimport MODULE ${kritasvgimport_SOURCES})

add_definitions(${SVG_DEFINITIONS})

target_link_libraries(kritasvgimport kritaui)

install(TARGETS kritasvgimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_svg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

set(kritasvgexport_SOURCES
    kis_scalable_vector_graphics_save_context.cpp
    kis_scalable_vector_graphics_save_visitor.cpp
    kis_svg_export.cpp
    )

kis_add_library(kritasvgexport MODULE ${kritasvgexport_SOURCES})

target_link_libraries(kritasvgexport kritaui kritaimpex)

install(TARGETS kritasvgexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install(PROGRAMS  krita_svg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
