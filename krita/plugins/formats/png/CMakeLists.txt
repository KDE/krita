add_subdirectory(tests)

include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata )

set(kritapngimport_PART_SRCS
    kis_png_import.cc
    )

add_library(kritapngimport MODULE ${kritapngimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritapngimport krita_png_import.desktop)


include_directories(${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritapngimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritapngimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritapngexport_PART_SRCS
    kis_png_export.cc
    )


ki18n_wrap_ui(kritapngexport_PART_SRCS kis_wdg_options_png.ui )

add_library(kritapngexport MODULE ${kritapngexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritapngexport krita_png_export.desktop)


target_link_libraries(kritapngexport kritaui ${PNG_LIBRARIES})

install(TARGETS kritapngexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_png.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
