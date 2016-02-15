add_subdirectory(tests)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(kritappmimport_SOURCES
    kis_ppm_import.cpp
    )

add_library(kritappmimport MODULE ${kritappmimport_SOURCES})

target_link_libraries(kritappmimport kritaui )

install(TARGETS kritappmimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritappmexport_SOURCES
    kis_ppm_export.cpp
    )

ki18n_wrap_ui(kritappmexport_SOURCES kis_wdg_options_ppm.ui )

add_library(kritappmexport MODULE ${kritappmexport_SOURCES})

target_link_libraries(kritappmexport kritaui  )

install(TARGETS kritappmexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_ppm.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
