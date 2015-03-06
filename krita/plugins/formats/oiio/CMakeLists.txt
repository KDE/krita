set(kritaoiioexport_PART_SRCS
    kis_oiio_export.cpp
    )

ki18n_wrap_ui(kritaoiioexport_PART_SRCS )

add_library(kritaoiioexport MODULE ${kritaoiioexport_PART_SRCS})

target_link_libraries(kritaoiioexport kritaui ${OIIO_LIBRARIES})

install(TARGETS kritaoiioexport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_oiio_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_oiio.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

set(kritaoiioimport_PART_SRCS
    kis_oiio_import.cpp
    )

ki18n_wrap_ui(kritaoiioimport_PART_SRCS )

add_library(kritaoiioimport MODULE ${kritaoiioimport_PART_SRCS})

target_link_libraries(kritaoiioimport kritaui ${OIIO_LIBRARIES})

install(TARGETS kritaoiioimport  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  krita_oiio_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( PROGRAMS  krita_oiio.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
