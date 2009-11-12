add_subdirectory(tests)

include_directories( ${KRITA_INCLUDES} ${CMAKE_CURRENT_fSOURCE_DIR} )

set(kritappmimport_PART_SRCS
    kis_ppm_import.cpp
    )

kde4_add_plugin(kritappmimport WITH_PREFIX ${kritappmimport_PART_SRCS})

target_link_libraries(kritappmimport kritaui )

install(TARGETS kritappmimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritappmexport_PART_SRCS
    kis_ppm_export.cpp
    )

kde4_add_ui_files(kritappmexport_PART_SRCS kis_wdg_options_ppm.ui )

kde4_add_plugin(kritappmexport WITH_PREFIX ${kritappmexport_PART_SRCS})

target_link_libraries(kritappmexport kritaui  )

install(TARGETS kritappmexport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  krita_ppm_import.desktop krita_ppm_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita_ppm.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
