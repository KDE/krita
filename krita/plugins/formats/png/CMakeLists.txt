add_subdirectory(tests)

include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata )

set(kritapngimport_SOURCES
    kis_png_import.cc
    )

add_library(kritapngimport MODULE ${kritapngimport_SOURCES})

include_directories(${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritapngimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritapngimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritapngexport_SOURCES
    kis_png_export.cc
)

ki18n_wrap_ui(kritapngexport_SOURCES kis_wdg_options_png.ui )

add_library(kritapngexport MODULE ${kritapngexport_SOURCES})

target_link_libraries(kritapngexport kritaui ${PNG_LIBRARIES})

install(TARGETS kritapngexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_png.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
