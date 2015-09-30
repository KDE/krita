set(kritatgaexport_PART_SRCS
    kis_tga_export.cpp
    )

ki18n_wrap_ui(kritatgaexport_PART_SRCS )

add_library(kritatgaexport MODULE ${kritatgaexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritatgaexport krita_tga_export.desktop)

target_link_libraries(kritatgaexport kritaui)

install(TARGETS kritatgaexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

set(kritatgaimport_PART_SRCS
    kis_tga_import.cpp
    )

ki18n_wrap_ui(kritatgaimport_PART_SRCS )

add_library(kritatgaimport MODULE ${kritatgaimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritatgaimport krita_tga_import.desktop)

target_link_libraries(kritatgaimport kritaui )

install(TARGETS kritatgaimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( PROGRAMS  krita_tga.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
