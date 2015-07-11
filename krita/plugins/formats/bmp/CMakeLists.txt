set(kritabmpexport_PART_SRCS
    kis_bmp_export.cpp
    )

ki18n_wrap_ui(kritabmpexport_PART_SRCS )

add_library(kritabmpexport MODULE ${kritabmpexport_PART_SRCS})
kcoreaddons_desktop_to_json(kritabmpexport krita_bmp_export.desktop)

target_link_libraries(kritabmpexport kritaui)

install(TARGETS kritabmpexport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES  krita_bmp_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
set(kritabmpimport_PART_SRCS
    kis_bmp_import.cpp
    )

ki18n_wrap_ui(kritabmpimport_PART_SRCS )

add_library(kritabmpimport MODULE ${kritabmpimport_PART_SRCS})
kcoreaddons_desktop_to_json(kritabmpimport krita_bmp_import.desktop)

target_link_libraries(kritabmpimport kritaui ${KDE4_KIO_LIBS} )

install(TARGETS kritabmpimport  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES  krita_bmp_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_bmp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
