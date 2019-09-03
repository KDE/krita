add_subdirectory(tests)

set(kritapngimport_SOURCES
    kis_png_import.cc
    )

kis_add_library(kritapngimport MODULE ${kritapngimport_SOURCES})

include_directories(SYSTEM ${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS})

target_link_libraries(kritapngimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritapngimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritapngexport_SOURCES
    kis_png_export.cc
)

ki18n_wrap_ui(kritapngexport_SOURCES kis_wdg_options_png.ui )

kis_add_library(kritapngexport MODULE ${kritapngexport_SOURCES})

target_link_libraries(kritapngexport kritaui kritaimpex ${PNG_LIBRARIES})

install(TARGETS kritapngexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_png.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
