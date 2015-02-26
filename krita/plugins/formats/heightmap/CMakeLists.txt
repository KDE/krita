include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(kritaheightmapimport_PART_SRCS
    kis_heightmap_import.cpp
    )

kde4_add_ui_files(kritaheightmapimport_PART_SRCS kis_wdg_options_heightmap.ui )

kde4_add_plugin(kritaheightmapimport ${kritaheightmapimport_PART_SRCS})

target_link_libraries(kritaheightmapimport kritaui )

install(TARGETS kritaheightmapimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritaheightmapexport_PART_SRCS
    kis_heightmap_export.cpp
    )

kde4_add_ui_files(kritaheightmapexport_PART_SRCS kis_wdg_options_heightmap.ui )

kde4_add_plugin(kritaheightmapexport ${kritaheightmapexport_PART_SRCS})

target_link_libraries(kritaheightmapexport kritaui  )

install(TARGETS kritaheightmapexport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  krita_heightmap_import.desktop krita_heightmap_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_heightmap.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
