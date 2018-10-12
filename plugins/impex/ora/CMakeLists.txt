set(libkritaconverter_LIB_SRCS
    ora_converter.cpp
    kis_open_raster_load_context.cpp
    kis_open_raster_save_context.cpp
    kis_open_raster_stack_load_visitor.cpp
    kis_open_raster_stack_save_visitor.cpp
    )

set(kritaoraimport_SOURCES
    ora_import.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaoraimport MODULE ${kritaoraimport_SOURCES})

target_link_libraries(kritaoraimport kritaui )

install(TARGETS kritaoraimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

set(kritaoraexport_SOURCES
    ora_export.cc
    ${libkritaconverter_LIB_SRCS}
    )

add_library(kritaoraexport MODULE ${kritaoraexport_SOURCES})

target_link_libraries(kritaoraexport kritaui kritaimpex)

install(TARGETS kritaoraexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_ora.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

