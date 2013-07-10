add_subdirectory(tests)

include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata )

set(kritapngimport_PART_SRCS
    kis_png_import.cc
    )

kde4_add_plugin(kritapngimport ${kritapngimport_PART_SRCS})



include_directories(${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritapngimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritapngimport  DESTINATION ${PLUGIN_INSTALL_DIR})

set(kritapngexport_PART_SRCS
    kis_png_export.cc
    )


kde4_add_ui_files(kritapngexport_PART_SRCS kis_wdg_options_png.ui )

kde4_add_plugin(kritapngexport ${kritapngexport_PART_SRCS})



target_link_libraries(kritapngexport kritaui ${PNG_LIBRARIES})

install(TARGETS kritapngexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_png_import.desktop krita_png_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_png.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
