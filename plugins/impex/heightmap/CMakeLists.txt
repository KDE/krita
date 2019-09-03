add_subdirectory(tests)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(kritaheightmapimport_SOURCES
    kis_heightmap_import.cpp
    kis_wdg_options_heightmap.cpp
    kis_heightmap_utils.cpp
    )

ki18n_wrap_ui(kritaheightmapimport_SOURCES kis_wdg_options_heightmap.ui )

kis_add_library(kritaheightmapimport MODULE ${kritaheightmapimport_SOURCES})

target_link_libraries(kritaheightmapimport kritaui )

install(TARGETS kritaheightmapimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaheightmapexport_SOURCES
    kis_heightmap_export.cpp
    kis_wdg_options_heightmap.cpp
    kis_heightmap_utils.cpp
    )

ki18n_wrap_ui(kritaheightmapexport_SOURCES kis_wdg_options_heightmap.ui )

kis_add_library(kritaheightmapexport MODULE ${kritaheightmapexport_SOURCES})

target_link_libraries(kritaheightmapexport kritaui kritaimpex)

install(TARGETS kritaheightmapexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_heightmap.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
