add_subdirectory(tests)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(kritappmimport_PART_SRCS
    kis_ppm_import.cpp
    )

add_library(kritappmimport MODULE ${kritappmimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritappmimport krita_ppm_import.desktop)

target_link_libraries(kritappmimport kritaui )

install(TARGETS kritappmimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritappmexport_PART_SRCS
    kis_ppm_export.cpp
    )

ki18n_wrap_ui(kritappmexport_PART_SRCS kis_wdg_options_ppm.ui )

add_library(kritappmexport MODULE ${kritappmexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritappmexport krita_ppm_export.desktop)

target_link_libraries(kritappmexport kritaui  )

install(TARGETS kritappmexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( PROGRAMS  krita_ppm.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
